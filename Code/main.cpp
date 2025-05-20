#include <iostream>
#include <string>
#include <iostream>
#include <tclap/CmdLine.h>
#include <fstream>
#include <sstream>
#include "graph.h"

using namespace std;

int main(int argc, char** argv) {

  try{
    TCLAP::CmdLine cmd("Program Description", ' ', "1.0");
    TCLAP::ValueArg<std::string> inputGraph("i", "input", "path of the temporal graph edge list", true, "", "string");
    TCLAP::ValueArg<int> num_vertice("n", "num_vertice", "the number of vertices", true, 0, "int");
    TCLAP::ValueArg<int> num_edge("m", "num_edge", "all number of edges", true, 0, "int");
    TCLAP::ValueArg<int> num_update_edge("u", "num_update_edge", "the number of update edges", false, 0, "int");
    TCLAP::ValueArg<int> time_type("t", "time_type", "time type", false, 0, "int");
    TCLAP::ValueArg<int> window("w", "window", "time window", true, 0, "int");
    TCLAP::ValueArg<int> algo("a", "algo", "algorithm type", false, 0, "int");
    TCLAP::ValueArg<bool> reverse("r", "reverse", "reverse Direction of edge", false, true, "bool");
    TCLAP::ValueArg<std::string> TimeResult("e", "time_file", "path of time result file", false, "Time_Ours", "string");
    // TCLAP::ValueArg<std::string> PathResult("p", "update_file", "path of path result file", false, "Update_Result", "string");

    cmd.add(inputGraph);
    cmd.add(num_vertice);
    cmd.add(num_edge);
    cmd.add(num_update_edge);
    cmd.add(time_type);
    cmd.add(window);
    cmd.add(algo);
    cmd.add(reverse);
    cmd.add(TimeResult);
    // cmd.add(PathResult);

    // Parse the argv array.
    cmd.parse(argc, argv);

    Graph graph;
    string s = "/root/CycleEnum/Data/" + inputGraph.getValue() + ".txt";
    // string s = inputGraph.getValue();
    graph.num_vertices_ = num_vertice.getValue();
    graph.all_edges_num_ = num_edge.getValue();
    graph.update_edges_place = graph.all_edges_num_ - num_update_edge.getValue();
    // cout << graph.num_vertices_ << " " << graph.all_edges_num_ << " " << graph.update_edges_place  << endl;
    if(time_type.getValue() == 0){
        graph.window_size = window.getValue() * 60 * 60;
    }else if(time_type.getValue() == 1){
        graph.window_size = window.getValue() * 60;
    }else if(time_type.getValue() == 2){
        graph.window_size = window.getValue();
    }
    bool dir = reverse.getValue();
    int Algo = algo.getValue();
    string TimeResultFile = "/root/CycleEnum/Result/Time/" + TimeResult.getValue() + ".txt";
    // string PathResultFile = "/root/CycleEnum/Result/Update/" + PathResult.getValue();

    Timer Total_time;
    Total_time.reset();
    switch (Algo) {
        case 1:
            graph.load_graph(s);
            graph.Filter(dir);
            break;
        case 2:
            graph.load_graph(s);
            graph.Block(dir);
            break;
        case 3:
            graph.update(inputGraph.getValue());
            break;
        case 4:
            graph.update_naive(inputGraph.getValue());
            break;
        case 5:
            graph.Throughput_and_tail(inputGraph.getValue());
            break;
        default:
            std::cout << "Please enter the algorithm type!" << endl;
            return 0;
    }

    // graph.Path_Result_Output(PathResultFile);
    // std::cout << "DATA: " << inputGraph.getValue() << "ALGORITHM: " << Algo << ", DIRECTION: " << dir << endl;
    // std::cout << "--Cycle Number: " << graph.cycle_number << endl;
    // std::cout << "--Time Cost: (ms) " << graph.running_time.elapsedMilliseconds() << endl;
    // std::cout << "--Max Cycle Length: " << graph.max_cycle_length << endl;

    if (Algo == 1 || Algo == 2) {
        ofstream Result;
        Result.open(TimeResultFile.c_str(), ios::app);
        Result << "数据集: " << inputGraph.getValue() << endl;
        Result << "时间窗口: " << graph.window_size << endl;
        Result << "算法类型: " << Algo << endl;
        Result << "环路数量: " << graph.cycle_number << endl;
        Result << "最大环路长度: " << graph.max_cycle_length << endl;
        Result << "运行时间: " << Total_time.elapsedMilliseconds() << endl;
        Result << endl;
    }

  } catch (TCLAP::ArgException &e)  // catch any exceptions
  { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
  catch (std::exception &e) {
      std::cerr << "error: " << e.what() << std::endl;
  }
  return 0;
}
