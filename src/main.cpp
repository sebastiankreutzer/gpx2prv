#include <fstream>
#include <iostream>
#include <string>

#include "gpx2prv/gpx.hpp"
#include "gpx2prv/prv.hpp"

static void generate_cfg(const std::string& output_prefix, int num_threads) {
  std::string cfg_file = output_prefix + ".cfg";
  std::ofstream cfg(cfg_file);

  cfg << "#ParaverCFG\n";
  cfg << "ConfigFile.Version: 3.4\n";
  cfg << "ConfigFile.NumWindows: 1\n";
  cfg << "ConfigFile.BeginDescription\n";
  cfg << "GPX Route Visualization - Elevation by latitude band\n";
  cfg << "ConfigFile.EndDescription\n";
  cfg << "\n";
  cfg << "################################################################################\n";
  cfg << "< NEW DISPLAYING WINDOW GPX Route Elevation >\n";
  cfg << "################################################################################\n";
  cfg << "window_name GPX Route Elevation\n";
  cfg << "window_type single\n";
  cfg << "window_position_x 100\n";
  cfg << "window_position_y 100\n";
  cfg << "window_width 1200\n";
  cfg << "window_height 800\n";
  cfg << "window_comm_lines_enabled false\n";
  cfg << "window_flags_enabled false\n";
  cfg << "window_noncolor_mode false\n";
  cfg << "window_custom_color_enabled false\n";
  cfg << "window_semantic_scale_min_at_zero false\n";
  cfg << "window_logical_filtered true\n";
  cfg << "window_physical_filtered false\n";
  cfg << "window_intracomms_enabled true\n";
  cfg << "window_intercomms_enabled true\n";
  cfg << "window_comm_fromto true\n";
  cfg << "window_comm_tagsize true\n";
  cfg << "window_comm_typeval true\n";
  cfg << "window_maximum_y 0\n";
  cfg << "window_minimum_y 0\n";
  cfg << "window_compute_y_max true\n";
  cfg << "window_level thread\n";
  cfg << "window_scale_relative 1.000000000000\n";
  cfg << "window_end_time_relative 1.000000000000\n";
  cfg << "window_object appl { 1, { 1 } }\n";
  cfg << "window_begin_time_relative 0.000000000000\n";
  cfg << "window_open true\n";
  cfg << "window_drawmode draw_last\n";
  cfg << "window_drawmode_rows draw_last\n";
  cfg << "window_pixel_size 1\n";
  cfg << "window_labels_to_draw 1\n";
  cfg << "window_object_axis_position 0\n";
  cfg << "window_selected_functions { 14, { {cpu, Active Thd}, {appl, Adding}, "
          "{task, Adding}, {thread, State As Is}, {node, Adding}, {system, "
          "Adding}, {workload, Adding}, {from_obj, All}, {to_obj, All}, "
          "{tag_msg, All}, {size_msg, All}, {bw_msg, All}, {evt_type, "
          "All}, {evt_value, All} } }\n";
  cfg << "window_compose_functions { 9, { {compose_cpu, As Is}, "
          "{compose_appl, As Is}, {compose_task, As Is}, {compose_thread, As "
          "Is}, {compose_node, As Is}, {compose_system, As Is}, "
          "{compose_workload, As Is}, {topcompose1, As Is}, {topcompose2, As "
          "Is} } }\n";
  cfg << "window_filter_module evt_type 0\n";

  cfg.close();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <gpx_file> [output_prefix] [--map]"
              << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  --map  Generate map view (longitude→X-axis, latitude→Y-axis)"
              << std::endl;
    return 1;
  }

  std::string input_file = argv[1];
  std::string output_prefix = "output";
  bool map_mode = false;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--map") {
      map_mode = true;
    } else if (arg.rfind("--", 0) != 0) {
      output_prefix = arg;
    }
  }

  auto result = gpx::parse_file(input_file);

  if (!result.success) {
    std::cerr << "Error parsing GPX: " << result.error_message << std::endl;
    return 1;
  }

  prv::Options opts;
  opts.num_lat_bands = 100;
  opts.show_elevation = true;
  opts.connect_points = true;
  opts.map_mode = map_mode;

  prv::convert(result.document, output_prefix, opts);

  if (map_mode) {
    std::string map_cfg = output_prefix + "_map.cfg";
    std::ofstream cfg(map_cfg);
    cfg << "#ParaverCFG\n";
    cfg << "ConfigFile.Version: 3.4\n";
    cfg << "ConfigFile.NumWindows: 1\n";
    cfg << "ConfigFile.BeginDescription\n";
    cfg << "GPX Route Visualization - Map View (lon→X, lat→Y)\n";
    cfg << "ConfigFile.EndDescription\n";
    cfg << "\n";
    cfg << "################################################################################\n";
    cfg << "< NEW DISPLAYING WINDOW GPX Map View >\n";
    cfg << "################################################################################\n";
    cfg << "window_name GPX Map View\n";
    cfg << "window_type single\n";
    cfg << "window_position_x 100\n";
    cfg << "window_position_y 100\n";
    cfg << "window_width 1200\n";
    cfg << "window_height 800\n";
    cfg << "window_comm_lines_enabled false\n";
    cfg << "window_flags_enabled false\n";
    cfg << "window_noncolor_mode false\n";
    cfg << "window_custom_color_enabled false\n";
    cfg << "window_semantic_scale_min_at_zero false\n";
    cfg << "window_logical_filtered true\n";
    cfg << "window_physical_filtered false\n";
    cfg << "window_intracomms_enabled true\n";
    cfg << "window_intercomms_enabled true\n";
    cfg << "window_comm_fromto true\n";
    cfg << "window_comm_tagsize true\n";
    cfg << "window_comm_typeval true\n";
    cfg << "window_maximum_y 0\n";
    cfg << "window_minimum_y 0\n";
    cfg << "window_compute_y_max true\n";
    cfg << "window_level thread\n";
    cfg << "window_scale_relative 1.000000000000\n";
    cfg << "window_end_time_relative 1.000000000000\n";
    cfg << "window_object appl { 1, { 1 } }\n";
    cfg << "window_begin_time_relative 0.000000000000\n";
    cfg << "window_open true\n";
    cfg << "window_drawmode draw_last\n";
    cfg << "window_drawmode_rows draw_last\n";
    cfg << "window_pixel_size 1\n";
    cfg << "window_labels_to_draw 1\n";
    cfg << "window_object_axis_position 0\n";
    cfg << "window_color_mode code\n";
    cfg << "window_selected_functions { 14, { {cpu, Active Thd}, {appl, Adding}, "
            "{task, Adding}, {thread, State As Is}, {node, Adding}, {system, "
            "Adding}, {workload, Adding}, {from_obj, All}, {to_obj, All}, "
            "{tag_msg, All}, {size_msg, All}, {bw_msg, All}, {evt_type, "
            "All}, {evt_value, All} } }\n";
    cfg << "window_compose_functions { 9, { {compose_cpu, As Is}, "
            "{compose_appl, As Is}, {compose_task, As Is}, {compose_thread, As "
            "Is}, {compose_node, As Is}, {compose_system, As Is}, "
            "{compose_workload, As Is}, {topcompose1, As Is}, {topcompose2, As "
            "Is} } }\n";
    cfg << "window_filter_module evt_type 0\n";
    cfg.close();

    generate_cfg(output_prefix, opts.num_lat_bands);
  } else {
    generate_cfg(output_prefix, opts.num_lat_bands);
  }

  std::cout << "Generated Paraver trace files:" << std::endl;
  if (map_mode) {
    std::cout << "  " << output_prefix << "_map.prv" << std::endl;
    std::cout << "  " << output_prefix << "_map.pcf" << std::endl;
    std::cout << "  " << output_prefix << "_map.row" << std::endl;
    std::cout << "  " << output_prefix << "_map.cfg" << std::endl;
  } else {
    std::cout << "  " << output_prefix << ".prv" << std::endl;
    std::cout << "  " << output_prefix << ".pcf" << std::endl;
    std::cout << "  " << output_prefix << ".row" << std::endl;
    std::cout << "  " << output_prefix << ".cfg" << std::endl;
  }
  std::cout << std::endl;

  if (map_mode) {
    std::cout << "Map view: wxparaver " << output_prefix
              << "_map.prv " << output_prefix << "_map.cfg" << std::endl;
  } else {
    std::cout << "Open with Paraver: wxparaver " << output_prefix << ".prv"
              << " " << output_prefix << ".cfg" << std::endl;
  }

  return 0;
}
