#ifndef GPX2PRV_PRV_HPP
#define GPX2PRV_PRV_HPP

#include "gpx.hpp"

#include <string>

namespace prv {

struct Options {
  int num_lat_bands = 100;
  bool show_elevation = true;
  bool connect_points = true;
  double elevation_scale = 1.0;
  bool map_mode = false;
};

struct ElevationBands {
  double min_ele = 0.0;
  double max_ele = 1000.0;
  int num_bands = 20;
};

void convert(const gpx::GpxDocument& gpx, const std::string& output_prefix,
             const Options& options = {});

}  // namespace prv

#endif  // GPX2PRV_PRV_HPP
