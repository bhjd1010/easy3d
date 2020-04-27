/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include "depth_image.h"
#include <easy3d/fileio/resources.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/util/logging.h>


using namespace easy3d;

// This example shows how to create depth images from the rendering.

int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    DepthImage viewer("Tutorial_506_DepthMaps");

    // the point cloud file.
    const std::string file_name = resource::directory() + "/data/fountain/pointcloud.ply";
    Model *model = viewer.add_model(file_name, true);
    if (!model) {
        LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    auto drawable = model->get_points_drawable("vertices");
    drawable->set_point_size(5);
    drawable->set_property_coloring(State::VERTEX, "v:color");

    // Run the viewer
    return viewer.run();
}
