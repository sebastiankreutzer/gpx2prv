#ifndef GPX2PRV_GPX_HPP
#define GPX2PRV_GPX_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gpx {

struct TrackPoint {
  double latitude = 0.0;
  double longitude = 0.0;
  double elevation = 0.0;
  std::string timestamp;
};

struct TrackSegment {
  std::vector<TrackPoint> points;
};

struct Track {
  std::string name;
  std::vector<TrackSegment> segments;
};

struct RoutePoint {
  double latitude = 0.0;
  double longitude = 0.0;
  double elevation = 0.0;
};

struct Route {
  std::string name;
  std::vector<RoutePoint> points;
};

struct Metadata {
  std::string name;
  std::string description;
};

struct GpxDocument {
  std::string version;
  std::string creator;
  Metadata metadata;
  std::vector<Track> tracks;
  std::vector<Route> routes;
};

struct ParseResult {
  bool success = false;
  std::string error_message;
  GpxDocument document;
};

ParseResult parse_file(const std::string& filepath);
ParseResult parse_string(const std::string& content);

}  // namespace gpx

#endif  // GPX2PRV_GPX_HPP
