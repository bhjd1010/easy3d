/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include "viewer.h"
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/algo/collider.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>    // for the mouse buttons

using namespace easy3d;


CollisionViewer::CollisionViewer(const std::string &title)
        : Viewer(title)
        , model0_color_(0.8f, 1.0f, 0.8f)
        , model1_color_(0.8f, 0.8f, 1.0f)
{
    auto m0 = add_model(resource::directory() + "/data/bunny.ply");
    auto m1 = add_model(resource::directory() + "/data/mannequin.ply");
    auto mesh0 = dynamic_cast<SurfaceMesh*>(m0);
    auto mesh1 = dynamic_cast<SurfaceMesh*>(m1);
    if (mesh0 && mesh1) {
        // use the manipulator to transform the first model (visualization only and geometry is not changed)
        mesh0->set_manipulator(new Manipulator(mesh0));

        mesh0->add_face_property<vec3>("face_color", model0_color_);
        mesh0->renderer()->get_triangles_drawable("faces")->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::FACE, "face_color");

        mesh1->add_face_property<vec3>("face_color", model1_color_);
        mesh1->renderer()->get_triangles_drawable("faces")->set_coloring(easy3d::State::COLOR_PROPERTY, easy3d::State::FACE, "face_color");

        collider_ = new Collider(mesh0, mesh1);
        // detect and show the result when the viewer is visible
        detect();

        timer_.set_interval(50, [this]()->void {
            static std::mutex mutex; // ensures the previous changes are completed
            std::lock_guard<std::mutex> guard(mutex);
            static float angle = 0.0f;
            angle += 0.0001f;
            auto T = mat4::rotation(vec3(0, 0, 1), angle) * models_[0]->manipulator()->frame()->matrix();
            models_[0]->manipulator()->frame()->setFromMatrix(T);
            detect();
        });
    }
    else
        LOG(WARNING) << "not two meshes have been loaded";
}


CollisionViewer::~CollisionViewer() {
    timer_.stop();
    delete collider_;
}


std::string CollisionViewer::usage() const {
    return ("--------------------- Collision Viewer usage ----------------------\n"
            "On start, you will see an animation. Press the 'space' key to stop \n"
            "the animation. Then you can using your mouse to manipulate the     \n"
            "bunny model to perform real-time collection detection:             \n"
            "    - ALT + left button: rotate bunny model                        \n"
            "    - ALT + right button: translate bunny model                    \n"
            "------------------------------------------------------------------ \n");
}


bool CollisionViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    if (collider_ && models().size() == 2 && modifiers == GLFW_MOD_ALT && timer_.is_stopped()) {
        auto manipulator = models_[0]->manipulator();
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            manipulator->frame()->action_rotate(x, y, dx, dy, camera_, ManipulatedFrame::NONE);
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            manipulator->frame()->action_translate(x, y, dx, dy, camera_, ManipulatedFrame::NONE);
        else
            return false;

        detect();
        return true;
    }
    else
        return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}


bool CollisionViewer::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
        timer_.stop();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void CollisionViewer::detect() {
    const auto pairs = collider_->detect(models_[0]->manipulator()->matrix(), mat4::identity());
    //std::cout << pairs.size() << " pairs of intersecting faces" << std::endl;

    // mark the intersecting faces red
    // Note: The following code is for visualizing the intersecting faces, and the code is not optimized.
    //      Ideas for better performance:
    //          (1) update only the color buffer;
    //          (2) use a shader storage buffer to transfer the *status* of the faces to the fragment shader.
    auto mesh0_colors = dynamic_cast<SurfaceMesh *>(models_[0])->get_face_property<vec3>("face_color");
    auto mesh1_colors = dynamic_cast<SurfaceMesh *>(models_[1])->get_face_property<vec3>("face_color");
    mesh0_colors.vector().assign(dynamic_cast<SurfaceMesh *>(models_[0])->n_faces(), model0_color_);
    mesh1_colors.vector().assign(dynamic_cast<SurfaceMesh *>(models_[1])->n_faces(), model1_color_);
    for (const auto &pair : pairs) {
        mesh0_colors[pair.first] = vec3(1.0f, 0.0f, 0.0f);
        mesh1_colors[pair.second] = vec3(1.0f, 0.0f, 0.0f);
    }
    models_[0]->renderer()->update();
    models_[1]->renderer()->update();

    update();
}
