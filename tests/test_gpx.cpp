#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "gpx2prv/gpx.hpp"

namespace {

std::string get_test_data_path(const std::string& filename) {
  return std::string(TESTS_DATA_DIR) + "/" + filename;
}

std::string read_file(const std::string& path) {
  std::ifstream file(path);
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

}  // namespace

class GpxParseTest : public ::testing::Test {
 protected:
  void SetUp() override {}
};

TEST_F(GpxParseTest, ParseMinimalGpx) {
  auto result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(result.success);
  EXPECT_EQ(result.document.version, "1.1");
  EXPECT_EQ(result.document.creator, "gpx2prv_test");
  EXPECT_EQ(result.document.tracks.size(), 1u);
  EXPECT_EQ(result.document.tracks[0].name, "Test Track");
  EXPECT_EQ(result.document.metadata.name, "Minimal Track");
}

TEST_F(GpxParseTest, ParseTrackWithElevation) {
  auto result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(result.success);

  const auto& track = result.document.tracks[0];
  ASSERT_EQ(track.segments.size(), 1u);
  ASSERT_EQ(track.segments[0].points.size(), 3u);

  EXPECT_EQ(track.segments[0].points[0].latitude, 40.0);
  EXPECT_EQ(track.segments[0].points[0].longitude, -74.0);
  EXPECT_EQ(track.segments[0].points[0].elevation, 100.0);
  EXPECT_EQ(track.segments[0].points[0].timestamp, "2024-01-01T10:00:00Z");

  EXPECT_EQ(track.segments[0].points[2].elevation, 200.0);
}

TEST_F(GpxParseTest, ParseRoute) {
  auto result = gpx::parse_file(get_test_data_path("route.gpx"));
  ASSERT_TRUE(result.success);
  EXPECT_EQ(result.document.routes.size(), 1u);
  EXPECT_EQ(result.document.routes[0].name, "Test Route");
  EXPECT_EQ(result.document.routes[0].points.size(), 2u);
  EXPECT_EQ(result.document.routes[0].points[0].latitude, 35.0);
  EXPECT_EQ(result.document.routes[0].points[0].elevation, 200.0);
}

TEST_F(GpxParseTest, ParseMetadata) {
  auto result = gpx::parse_file(get_test_data_path("minimal.gpx"));
  ASSERT_TRUE(result.success);
  EXPECT_EQ(result.document.metadata.name, "Minimal Track");
  EXPECT_EQ(result.document.metadata.description, "A simple track for testing");
}

TEST_F(GpxParseTest, ParseMultiSegmentTrack) {
  auto result = gpx::parse_file(get_test_data_path("multi_segment.gpx"));
  ASSERT_TRUE(result.success);

  const auto& track = result.document.tracks[0];
  ASSERT_EQ(track.segments.size(), 2u);
  EXPECT_EQ(track.segments[0].points.size(), 2u);
  EXPECT_EQ(track.segments[1].points.size(), 2u);
}

TEST_F(GpxParseTest, ParseMultiTrack) {
  auto result = gpx::parse_file(get_test_data_path("multi_track.gpx"));
  ASSERT_TRUE(result.success);
  EXPECT_EQ(result.document.tracks.size(), 2u);
  EXPECT_EQ(result.document.tracks[0].name, "Track A");
  EXPECT_EQ(result.document.tracks[1].name, "Track B");
}

TEST_F(GpxParseTest, ParseInvalidXml) {
  auto result = gpx::parse_string("<not-gpx>invalid</not-gpx>");
  EXPECT_FALSE(result.success);
  EXPECT_FALSE(result.error_message.empty());
}

TEST_F(GpxParseTest, ParseNonGpx) {
  auto result = gpx::parse_string("<?xml version=\"1.0\"?><root>Hello</root>");
  EXPECT_FALSE(result.success);
  EXPECT_EQ(result.error_message, "Not a valid GPX file");
}

TEST_F(GpxParseTest, ParseString) {
  std::string gpx_content = read_file(get_test_data_path("minimal.gpx"));
  auto result = gpx::parse_string(gpx_content);
  ASSERT_TRUE(result.success);
  EXPECT_EQ(result.document.tracks.size(), 1u);
}

TEST_F(GpxParseTest, ParseElevationRange) {
  auto result = gpx::parse_file(get_test_data_path("elevation.gpx"));
  ASSERT_TRUE(result.success);

  const auto& track = result.document.tracks[0];
  const auto& points = track.segments[0].points;

  double min_ele = points[0].elevation;
  double max_ele = points[0].elevation;
  for (const auto& pt : points) {
    min_ele = std::min(min_ele, pt.elevation);
    max_ele = std::max(max_ele, pt.elevation);
  }

  EXPECT_EQ(min_ele, 0.0);
  EXPECT_EQ(max_ele, 1000.0);
}
