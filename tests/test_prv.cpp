#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>

#include "gpx2prv/gpx.hpp"
#include "gpx2prv/prv.hpp"

namespace {

std::string get_test_data_path(const std::string& filename) {
  return std::string(TESTS_DATA_DIR) + "/" + filename;
}

int count_lines(const std::string& filepath) {
  std::ifstream file(filepath);
  int count = 0;
  std::string line;
  while (std::getline(file, line)) {
    ++count;
  }
  return count;
}

int count_state_records(const std::string& filepath) {
  std::ifstream file(filepath);
  std::string line;
  int count = 0;
  while (std::getline(file, line)) {
    if (!line.empty() && line[0] == '1' && line.find(':') != std::string::npos) {
      ++count;
    }
  }
  return count;
}

bool has_event_type_section(const std::string& filepath) {
  std::ifstream file(filepath);
  std::string line;
  bool found_event_type = false;
  while (std::getline(file, line)) {
    if (line == "EVENT_TYPE" || line == "STATE_TYPE" || line == "STATES") {
      found_event_type = true;
      break;
    }
  }
  return found_event_type;
}

std::string get_prv_header(const std::string& filepath) {
  std::ifstream file(filepath);
  std::string line;
  if (std::getline(file, line)) {
    return line;
  }
  return "";
}

int count_threads_in_row(const std::string& filepath) {
  std::ifstream file(filepath);
  std::string line;
  int thread_count = 0;
  while (std::getline(file, line)) {
    if (line.find("Thread ") != std::string::npos) {
      ++thread_count;
    }
  }
  return thread_count;
}

}  // namespace

class PrvConvertTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {
    std::remove(output_prefix_.c_str());
    std::remove((output_prefix_ + ".prv").c_str());
    std::remove((output_prefix_ + ".pcf").c_str());
    std::remove((output_prefix_ + ".row").c_str());
    std::remove((output_prefix_ + "_map.prv").c_str());
    std::remove((output_prefix_ + "_map.pcf").c_str());
    std::remove((output_prefix_ + "_map.row").c_str());
    std::remove((output_prefix_ + "_map.cfg").c_str());
  }

  std::string output_prefix_ = "test_output";
};

TEST_F(PrvConvertTest, ConvertMinimalTrack) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.num_lat_bands = 100;
  opts.show_elevation = true;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string prv_path = output_prefix_ + ".prv";
  std::ifstream file(prv_path);
  ASSERT_TRUE(file.good());

  std::string header = get_prv_header(prv_path);
  EXPECT_TRUE(header.find("#Paraver") != std::string::npos);
  EXPECT_TRUE(header.find(":1(") != std::string::npos);
}

TEST_F(PrvConvertTest, ConvertTrackCreatesRow) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.num_lat_bands = 100;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string row_path = output_prefix_ + ".row";
  std::ifstream file(row_path);
  ASSERT_TRUE(file.good());

  int thread_count = count_threads_in_row(row_path);
  EXPECT_EQ(thread_count, 100);
}

TEST_F(PrvConvertTest, ConvertCreatesPcf) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string pcf_path = output_prefix_ + ".pcf";
  std::ifstream file(pcf_path);
  ASSERT_TRUE(file.good());

  EXPECT_TRUE(has_event_type_section(pcf_path));
}

TEST_F(PrvConvertTest, ConvertElevationColorBands) {
  auto gpx_result = gpx::parse_file(get_test_data_path("elevation.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.show_elevation = true;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string pcf_path = output_prefix_ + ".pcf";
  std::ifstream file(pcf_path);
  ASSERT_TRUE(file.good());

  std::string line;
  bool found_elevation_0 = false;
  bool found_elevation_1000 = false;
  while (std::getline(file, line)) {
    if (line.find("Elevation: 0.0m") != std::string::npos) {
      found_elevation_0 = true;
    }
    if (line.find("Elevation: 1000.0m") != std::string::npos) {
      found_elevation_1000 = true;
    }
  }
  EXPECT_TRUE(found_elevation_0);
  EXPECT_TRUE(found_elevation_1000);
}

TEST_F(PrvConvertTest, ConvertMultipleTracks) {
  auto gpx_result = gpx::parse_file(get_test_data_path("multi_track.gpx"));
  ASSERT_TRUE(gpx_result.success);

  EXPECT_EQ(gpx_result.document.tracks.size(), 2u);

  prv::Options opts;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string prv_path = output_prefix_ + ".prv";
  int state_count = count_state_records(prv_path);
  EXPECT_EQ(state_count, 4);
}

TEST_F(PrvConvertTest, ConvertEmptyGpx) {
  gpx::GpxDocument empty_doc;
  empty_doc.version = "1.1";
  empty_doc.creator = "test";

  prv::Options opts;
  prv::convert(empty_doc, output_prefix_, opts);

  std::string prv_path = output_prefix_ + ".prv";
  std::ifstream file(prv_path);
  ASSERT_TRUE(file.good());

  std::string header = get_prv_header(prv_path);
  EXPECT_TRUE(header.find("#Paraver") != std::string::npos);
}

TEST_F(PrvConvertTest, ConvertSinglePoint) {
  auto gpx_result = gpx::parse_file(get_test_data_path("single_point.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string prv_path = output_prefix_ + ".prv";
  int state_count = count_state_records(prv_path);
  EXPECT_EQ(state_count, 1);
}

TEST_F(PrvConvertTest, ConvertMultiSegmentTrack) {
  auto gpx_result = gpx::parse_file(get_test_data_path("multi_segment.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string prv_path = output_prefix_ + ".prv";
  int state_count = count_state_records(prv_path);
  EXPECT_EQ(state_count, 4);
}

TEST_F(PrvConvertTest, PrvHeaderFormat) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.num_lat_bands = 50;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string header = get_prv_header(output_prefix_ + ".prv");
  EXPECT_TRUE(header.find(":1(50):") != std::string::npos);
}

TEST_F(PrvConvertTest, RowContainsLatitudeInfo) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string row_path = output_prefix_ + ".row";
  std::ifstream file(row_path);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  EXPECT_TRUE(content.find("lat=") != std::string::npos);
}

TEST_F(PrvConvertTest, RouteConversion) {
  auto gpx_result = gpx::parse_file(get_test_data_path("route.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string prv_path = output_prefix_ + ".prv";
  int state_count = count_state_records(prv_path);
  EXPECT_EQ(state_count, 2);
}

TEST_F(PrvConvertTest, PcfElevationValues) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.show_elevation = true;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string pcf_path = output_prefix_ + ".pcf";
  std::ifstream file(pcf_path);
  std::string line;
  int elevation_lines = 0;
  while (std::getline(file, line)) {
    if (line.find("Elevation:") != std::string::npos) {
      ++elevation_lines;
    }
  }
  EXPECT_EQ(elevation_lines, 20);
}

TEST_F(PrvConvertTest, MapModeCreatesMapFiles) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.map_mode = true;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string map_prv = output_prefix_ + "_map.prv";
  std::string map_pcf = output_prefix_ + "_map.pcf";
  std::string map_row = output_prefix_ + "_map.row";

  ASSERT_TRUE(std::ifstream(map_prv).good());
  ASSERT_TRUE(std::ifstream(map_pcf).good());
  ASSERT_TRUE(std::ifstream(map_row).good());
}

TEST_F(PrvConvertTest, MapModeDoesNotCreateDefaultFiles) {
  auto gpx_result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.map_mode = true;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string default_prv = output_prefix_ + ".prv";
  std::string default_pcf = output_prefix_ + ".pcf";
  std::string default_row = output_prefix_ + ".row";

  EXPECT_FALSE(std::ifstream(default_prv).good());
  EXPECT_FALSE(std::ifstream(default_pcf).good());
  EXPECT_FALSE(std::ifstream(default_row).good());
}

TEST_F(PrvConvertTest, MapModeRowContainsLatitudeInfo) {
  auto gpx_result = gpx::parse_file(get_test_data_path("elevation.gpx"));
  ASSERT_TRUE(gpx_result.success);

  prv::Options opts;
  opts.map_mode = true;
  prv::convert(gpx_result.document, output_prefix_, opts);

  std::string map_row = output_prefix_ + "_map.row";
  std::ifstream file(map_row);
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  EXPECT_TRUE(content.find("lat=") != std::string::npos);
}

TEST_F(PrvConvertTest, MapModeSegmentsVisible) {
  gpx::GpxDocument doc;
  doc.version = "1.1";
  doc.creator = "test";
  doc.tracks.push_back(gpx::Track());
  doc.tracks[0].segments.push_back(gpx::TrackSegment());
  doc.tracks[0].segments[0].points.push_back({41.0, 0.9, 100.0});
  doc.tracks[0].segments[0].points.push_back({41.1, 0.95, 200.0});
  doc.tracks[0].segments[0].points.push_back({41.2, 1.0, 300.0});

  prv::Options opts;
  opts.map_mode = true;
  opts.num_lat_bands = 10;
  prv::convert(doc, output_prefix_, opts);

  std::string map_prv = output_prefix_ + "_map.prv";
  std::ifstream file(map_prv);

  std::string line;
  std::vector<uint64_t> begin_times;
  std::vector<uint64_t> end_times;

  std::getline(file, line);
  while (std::getline(file, line)) {
    if (line.empty()) continue;
    int type, node, cpu, task, thread;
    uint64_t begin_time, end_time, state;
    char colon;
    std::istringstream iss(line);
    iss >> type >> colon >> node >> colon >> cpu >> colon >> task >> colon >> thread >> colon >> begin_time >> colon >> end_time >> colon >> state;
    begin_times.push_back(begin_time);
    end_times.push_back(end_time);
  }

  ASSERT_EQ(begin_times.size(), 3u);
  for (size_t i = 0; i < begin_times.size(); ++i) {
    EXPECT_LT(begin_times[i], end_times[i]) << "Segment " << i << " has no duration";
  }
}

TEST_F(PrvConvertTest, MapModeNoBackwardSegments) {
  gpx::GpxDocument doc;
  doc.version = "1.1";
  doc.creator = "test";
  doc.tracks.push_back(gpx::Track());
  doc.tracks[0].segments.push_back(gpx::TrackSegment());
  doc.tracks[0].segments[0].points.push_back({41.0, 1.0, 100.0});
  doc.tracks[0].segments[0].points.push_back({41.1, 0.5, 200.0});
  doc.tracks[0].segments[0].points.push_back({41.2, 0.1, 300.0});

  prv::Options opts;
  opts.map_mode = true;
  opts.num_lat_bands = 10;
  prv::convert(doc, output_prefix_, opts);

  std::string map_prv = output_prefix_ + "_map.prv";
  std::ifstream file(map_prv);

  std::string line;
  std::vector<uint64_t> begin_times;
  std::vector<uint64_t> end_times;

  std::getline(file, line);
  while (std::getline(file, line)) {
    if (line.empty()) continue;
    int type, node, cpu, task, thread;
    uint64_t begin_time, end_time, state;
    char colon;
    std::istringstream iss(line);
    iss >> type >> colon >> node >> colon >> cpu >> colon >> task >> colon >> thread >> colon >> begin_time >> colon >> end_time >> colon >> state;
    begin_times.push_back(begin_time);
    end_times.push_back(end_time);
  }

  ASSERT_EQ(begin_times.size(), 3u);
  for (size_t i = 0; i < begin_times.size(); ++i) {
    EXPECT_LT(begin_times[i], end_times[i])
        << "Segment " << i << " has zero or negative duration (westward segment must have positive duration)";
  }
}
