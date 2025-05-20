#ifndef DIRECTED_GRAPH_H
#define DIRECTED_GRAPH_H

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include "Timer.h"

const int MAX_NUMBER = 2147483647;
const int MAX_LENGTH = 100;

class Graph{
public:
    int num_vertices_;
    int num_edges_;
    int window_size;

    struct Edge{
        int node_id;
        int time;
        int edge_id;
    };

    struct Edge_info{
        int src;
        int dst;
        int time;
        int edge_id;
        int next_edge_start;
        int next_edge_end;
        int last_edge_start;
        int last_edge_end;
    };

    struct Block_node{
        int node_id;
        int node_time;
    };

    vector<vector<Edge>> out_graph;
    vector<vector<Edge>> in_graph;
    vector<Edge_info> all_edges;

    bool* visited; //check whether the path is a simple path
    int partial_path[MAX_LENGTH]; //recode the current path information
    int max_cycle_length = 0;
    int* each_result; //recore the number of the result path of length i 
    int cycle_number = 0;
    int add_cycle_number = 0;
    int cur_cycle_number = 0;

    Timer running_time;
    Timer dynamic_search_cost;
    Timer new_search_cost;

    std::set<std::string> cycle_path; //store the result;
    std::vector<std::string> add_cycle_path_by_dynamic;
    std::vector<std::string> add_cycle_path_by_new;

    //read graph;
    void read_graph(string graph_file);
    void reduce_graph();
    void reduce_in_graph(int node);
    void reduce_out_graph(int node);
    void edge_filter();
    void load_graph(string graph_file);
    void Test();

    //filter baseline;
    void DFS(int start_edge, int cur_depth, int cur_edge, bool dir);
    void Filter(bool dir);

    //block;
    int stamp = 0;
    vector<int> stamp_record; //check the whether the node is blocked;
    vector<int> close_time;
    vector<int> block_set_stamp_node; //check the unblock set;
    vector<std::vector<Block_node> > unblock_list;
    void unblock(int node, int time, bool dir);
    bool DFS_block(int start_edge, int cur_depth, int cur_edge, bool dir);
    void Block(bool dir);

    //update graph algorithm;
    int all_edges_num_; // all edges number;
    int update_edges_place; // the place of update edges begin;
    struct update_edge
    {
        int src;
        int dst;
        int time;
    };
    vector<update_edge> update_edge_list;

    void update_single_edge(int src, int dst, int time);
    bool DFS_block_dynamic(int start_edge, int cur_depth, int cur_edge, bool break_point, int break_place);
    void Dynamic_search(int start_edge);
    void load_update_edges(string update_file);
    void update(string update_file);
    void update_naive(string update_file);

    //compute the Throughput and tail Latency
    void Throughput_and_tail(string update_file);
    void Throughput_and_tail_naive(string update_file);

    //output result:
    void Path_Result_Output(string path_file);
};


#endif //DIRECTED_GRAPH_H