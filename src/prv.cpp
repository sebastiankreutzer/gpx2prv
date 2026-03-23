#include "gpx2prv/prv.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <sys/time.h>
#include <unordered_map>
#include <vector>

namespace {

std::string get_timestamp() {
  timeval tv;
  gettimeofday(&tv, nullptr);
  tm* tm_info = localtime(&tv.tv_sec);
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%d/%m/%y at %H:%M:%S", tm_info);
  return std::string(buffer);
}

double normalize_0_1(double value, double min_val, double max_val) {
  if (max_val <= min_val)
    return 0.5;
  return (value - min_val) / (max_val - min_val);
}

double lerp(double a, double b, double t) { return a + (b - a) * t; }

int latitude_to_thread(double lat, double lat_min, double lat_max,
                        int num_threads) {
  double t = normalize_0_1(lat, lat_min, lat_max);
  t = std::clamp(t, 0.0, 1.0);
  return static_cast<int>(t * (num_threads - 1));
}

int longitude_to_thread(double lon, double lon_min, double lon_max,
                         int num_threads) {
  double t = normalize_0_1(lon, lon_min, lon_max);
  t = std::clamp(t, 0.0, 1.0);
  return static_cast<int>(t * (num_threads - 1));
}

int elevation_to_state(double ele, double ele_min, double ele_max,
                       int num_states) {
  double t = normalize_0_1(ele, ele_min, ele_max);
  t = std::clamp(t, 0.0, 1.0);
  return static_cast<int>(t * (num_states - 1)) + 1;
}

std::string format_timestamp(uint64_t ns) {
  std::ostringstream ss;
  ss << ns;
  return ss.str();
}

struct TracePoint {
  int thread;
  uint64_t timestamp;
  uint64_t next_timestamp;
  double latitude;
  double longitude;
  double elevation;
};

struct GpxPoint {
  double latitude;
  double longitude;
  double elevation;
};

}  // namespace

namespace prv {

namespace {

void write_trace_files(const std::string& output_prefix,
                       const std::vector<TracePoint>& all_points,
                       double lat_min, double lat_max,
                       double lon_min, double lon_max,
                       double elevation_min, double elevation_max,
                       int num_threads, int num_ele_states,
                       uint64_t final_time, bool show_elevation,
                       bool is_map_mode,
                       uint64_t default_point_duration = 100000ULL) {
  std::string suffix = is_map_mode ? "_map" : "";
  std::string prv_file = output_prefix + suffix + ".prv";
  std::string pcf_file = output_prefix + suffix + ".pcf";
  std::string row_file = output_prefix + suffix + ".row";

  std::ofstream pcf(pcf_file);
  pcf << "DEFAULT_OPTIONS\n";
  pcf << "\n";
  pcf << "LEVEL               THREAD\n";
  pcf << "UNITS               NANOSEC\n";
  pcf << "LOOK_BACK           100\n";
  pcf << "SPEED               1\n";
  pcf << "FLAG_ICONS          ENABLED\n";
  pcf << "NUM_OF_STATE_COLORS " << num_ele_states << "\n";
  pcf << "YMAX_SCALE          " << num_threads << "\n";
  pcf << "\n";
  pcf << "\n";
  pcf << "DEFAULT_SEMANTIC\n";
  pcf << "\n";
  pcf << "THREAD_FUNC          State As Is\n";
  pcf << "\n";
  pcf << "\n";
  pcf << "STATES\n";
  for (int i = 0; i < num_ele_states; ++i) {
    double ele_val = lerp(elevation_min, elevation_max,
                          static_cast<double>(i) / (num_ele_states - 1));
    pcf << (i + 1) << "\tElevation: " << std::fixed
        << std::setprecision(1) << ele_val << "m\n";
  }
  pcf << "\n";
  pcf << "\n";
  pcf << "STATES_COLOR\n";
  for (int i = 0; i < num_ele_states; ++i) {
    double t = static_cast<double>(i) / (num_ele_states - 1);
    int r = static_cast<int>(255 * t);
    int g = static_cast<int>(255 * (1 - t));
    int b = 128;
    pcf << (i + 1) << "\t{" << r << "," << g << "," << b << "}\n";
  }
  pcf << "\n";
  pcf.close();

  std::ofstream row(row_file);
  row << "Thread: " << num_threads << "\n";
  for (int i = 0; i < num_threads; ++i) {
    double value;
    if (is_map_mode) {
      value = lerp(lat_max, lat_min, static_cast<double>(i) / (num_threads - 1));
    } else {
      value = lerp(lat_min, lat_max, static_cast<double>(i) / (num_threads - 1));
    }
    row << "  Thread " << i << ": lat=" << std::fixed
        << std::setprecision(6) << value << "\n";
  }
  row.close();

  std::ofstream prv(prv_file);
  prv << "#Paraver (" << get_timestamp()
      << "):" << format_timestamp(final_time) << "_ns:1("
      << num_threads << "):1:1(" << num_threads << ":1)\n";

  for (const auto& pt : all_points) {
    int state = show_elevation
                    ? elevation_to_state(pt.elevation, elevation_min,
                                         elevation_max, num_ele_states)
                    : 1;

    uint64_t end_time = (pt.next_timestamp > pt.timestamp) ? pt.next_timestamp : pt.timestamp + default_point_duration;
    prv << "1:1:1:1:" << (pt.thread + 1) << ":" << format_timestamp(pt.timestamp)
        << ":" << format_timestamp(end_time) << ":" << state << "\n";
  }

  prv.close();
}

}  // anonymous namespace

void convert(const gpx::GpxDocument& gpx, const std::string& output_prefix,
             const Options& options) {
  std::vector<TracePoint> all_points;
  std::vector<double> all_elevations;
  std::vector<GpxPoint> all_gpx_points;

  double lat_min = 90, lat_max = -90;
  double lon_min = 180, lon_max = -180;

  for (const auto& track : gpx.tracks) {
    for (const auto& segment : track.segments) {
      for (const auto& point : segment.points) {
        lat_min = std::min(lat_min, point.latitude);
        lat_max = std::max(lat_max, point.latitude);
        lon_min = std::min(lon_min, point.longitude);
        lon_max = std::max(lon_max, point.longitude);
        all_elevations.push_back(point.elevation);
        all_gpx_points.push_back({point.latitude, point.longitude, point.elevation});
      }
    }
  }

  for (const auto& route : gpx.routes) {
    for (const auto& point : route.points) {
      lat_min = std::min(lat_min, point.latitude);
      lat_max = std::max(lat_max, point.latitude);
      lon_min = std::min(lon_min, point.longitude);
      lon_max = std::max(lon_max, point.longitude);
      all_elevations.push_back(point.elevation);
      all_gpx_points.push_back({point.latitude, point.longitude, point.elevation});
    }
  }

  if (lat_min == lat_max) {
    lat_min -= 0.001;
    lat_max += 0.001;
  }
  if (lon_min == lon_max) {
    lon_min -= 0.001;
    lon_max += 0.001;
  }

  double elevation_min = 0, elevation_max = 100;
  if (!all_elevations.empty()) {
    elevation_min = *std::min_element(all_elevations.begin(),
                                       all_elevations.end());
    elevation_max = *std::max_element(all_elevations.begin(),
                                       all_elevations.end());
    if (elevation_min == elevation_max) {
      elevation_min -= 10;
      elevation_max += 10;
    }
  }

  int num_ele_states = 20;
  const uint64_t total_time_us = 1000000ULL;
  const uint64_t point_duration = 100000ULL;
  const uint64_t gap_duration = 50000ULL;
  const uint64_t map_point_duration = 1000ULL;

    int num_threads = options.num_lat_bands;

  if (options.map_mode) {
    const int num_lon_buckets = 1000;
    double lon_bucket_size = (lon_max - lon_min) / num_lon_buckets;
    if (lon_bucket_size < 0.0001) lon_bucket_size = 0.0001;

    std::unordered_map<int, int> bucket_counts;
    for (const auto& gpx_pt : all_gpx_points) {
      int lon_bucket = static_cast<int>((gpx_pt.longitude - lon_min) / lon_bucket_size);
      lon_bucket = std::max(0, std::min(num_lon_buckets - 1, lon_bucket));
      bucket_counts[lon_bucket]++;
    }

    std::unordered_map<int, int> bucket_offsets;
    uint64_t max_next_timestamp = 0;
    for (size_t i = 0; i < all_gpx_points.size(); ++i) {
      const auto& gpx_pt = all_gpx_points[i];

      int lon_bucket = static_cast<int>((gpx_pt.longitude - lon_min) / lon_bucket_size);
      lon_bucket = std::max(0, std::min(num_lon_buckets - 1, lon_bucket));

      int offset = bucket_offsets[lon_bucket]++;
      int bucket_width = total_time_us / num_lon_buckets;
      uint64_t timestamp = static_cast<uint64_t>(lon_bucket) * bucket_width + offset;
      uint64_t next_timestamp = timestamp + 1000;

      if (next_timestamp > max_next_timestamp) max_next_timestamp = next_timestamp;

      int thread = num_threads - 1 - latitude_to_thread(gpx_pt.latitude, lat_min, lat_max, num_threads);

      TracePoint tp;
      tp.thread = thread;
      tp.timestamp = timestamp;
      tp.next_timestamp = next_timestamp;
      tp.latitude = gpx_pt.latitude;
      tp.longitude = gpx_pt.longitude;
      tp.elevation = gpx_pt.elevation;
      all_points.push_back(tp);
    }

    uint64_t final_time = max_next_timestamp + 1000;

    write_trace_files(output_prefix, all_points,
                      lat_min, lat_max, lon_min, lon_max,
                      elevation_min, elevation_max,
                      num_threads, num_ele_states, final_time,
                      options.show_elevation, true,
                      map_point_duration);
  } else {
    uint64_t current_time = 0;

    for (const auto& gpx_pt : all_gpx_points) {
      TracePoint tp;
      tp.thread = latitude_to_thread(gpx_pt.latitude, lat_min, lat_max,
                                    num_threads);
      tp.timestamp = current_time;
      tp.next_timestamp = 0;
      tp.latitude = gpx_pt.latitude;
      tp.longitude = gpx_pt.longitude;
      tp.elevation = gpx_pt.elevation;
      all_points.push_back(tp);

      current_time += point_duration;
      if (options.connect_points) {
        current_time += gap_duration;
      }
    }

    for (size_t i = 0; i < all_points.size(); ++i) {
      if (i + 1 < all_points.size()) {
        all_points[i].next_timestamp = all_points[i + 1].timestamp;
      }
    }

    uint64_t final_time = current_time + point_duration;
    if (all_points.empty()) {
      final_time = total_time_us;
    }

    write_trace_files(output_prefix, all_points,
                      lat_min, lat_max, lon_min, lon_max,
                      elevation_min, elevation_max,
                      num_threads, num_ele_states, final_time,
                      options.show_elevation, false,
                      point_duration);
  }
}

}  // namespace prv
