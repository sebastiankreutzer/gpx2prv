#include "gpx2prv/gpx.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <pugixml.hpp>

namespace {

std::string trim(const std::string& str) {
  size_t start = 0;
  while (start < str.size() &&
         std::isspace(static_cast<unsigned char>(str[start]))) {
    ++start;
  }
  size_t end = str.size();
  while (end > start &&
         std::isspace(static_cast<unsigned char>(str[end - 1]))) {
    --end;
  }
  return str.substr(start, end - start);
}

double parse_double(const std::string& str) {
  std::stringstream ss(trim(str));
  double value = 0.0;
  ss >> value;
  return value;
}

void parse_track_point(const pugi::xml_node& node, gpx::TrackPoint& point) {
  point.latitude = node.attribute("lat").as_double();
  point.longitude = node.attribute("lon").as_double();
  point.elevation = node.child_value("ele")[0] ? parse_double(node.child_value("ele")) : 0.0;
  point.timestamp = trim(node.child_value("time"));
}

void parse_track_segment(const pugi::xml_node& node, gpx::TrackSegment& segment) {
  for (const auto& pt_node : node.children("trkpt")) {
    gpx::TrackPoint point;
    parse_track_point(pt_node, point);
    segment.points.push_back(point);
  }
}

void parse_track(const pugi::xml_node& node, gpx::Track& track) {
  track.name = trim(node.child_value("name"));
  for (const auto& seg_node : node.children("trkseg")) {
    gpx::TrackSegment segment;
    parse_track_segment(seg_node, segment);
    track.segments.push_back(segment);
  }
}

void parse_route_point(const pugi::xml_node& node, gpx::RoutePoint& point) {
  point.latitude = node.attribute("lat").as_double();
  point.longitude = node.attribute("lon").as_double();
  point.elevation = node.child_value("ele")[0] ? parse_double(node.child_value("ele")) : 0.0;
}

void parse_route(const pugi::xml_node& node, gpx::Route& route) {
  route.name = trim(node.child_value("name"));
  for (const auto& pt_node : node.children("rtept")) {
    gpx::RoutePoint point;
    parse_route_point(pt_node, point);
    route.points.push_back(point);
  }
}

void parse_metadata(const pugi::xml_node& node, gpx::Metadata& metadata) {
  metadata.name = trim(node.child_value("name"));
  metadata.description = trim(node.child_value("desc"));
}

}  // namespace

namespace gpx {

static std::string read_file(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

ParseResult parse_file(const std::string& filepath) {
  ParseResult result;
  try {
    std::string content = read_file(filepath);
    result = parse_string(content);
  } catch (const std::exception& e) {
    result.success = false;
    result.error_message = e.what();
  }
  return result;
}

ParseResult parse_string(const std::string& content) {
  ParseResult result;
  pugi::xml_document doc;

  auto load_result = doc.load_string(content.c_str());
  if (!load_result) {
    result.success = false;
    result.error_message = load_result.description();
    return result;
  }

  auto gpx_node = doc.child("gpx");
  if (!gpx_node) {
    result.success = false;
    result.error_message = "Not a valid GPX file";
    return result;
  }

  result.document.version = gpx_node.attribute("version").as_string();
  result.document.creator = gpx_node.attribute("creator").as_string();

  auto metadata_node = gpx_node.child("metadata");
  if (metadata_node) {
    parse_metadata(metadata_node, result.document.metadata);
  }

  for (const auto& trk_node : gpx_node.children("trk")) {
    gpx::Track track;
    parse_track(trk_node, track);
    result.document.tracks.push_back(track);
  }

  for (const auto& rte_node : gpx_node.children("rte")) {
    gpx::Route route;
    parse_route(rte_node, route);
    result.document.routes.push_back(route);
  }

  result.success = true;
  return result;
}

}  // namespace gpx
