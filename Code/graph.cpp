#include "graph.h"

using namespace std;

void Graph::read_graph (string graph_path){
    out_graph.resize(num_vertices_);
    in_graph.resize(num_vertices_);

    ifstream infile;
    infile.open(graph_path.c_str());
    if (!infile.is_open()) {
        cerr << "错误：无法打开输入文件 " << graph_path << endl;
        return;
    }
    string line;
    int line_num = 0;
    Edge_info edge;
    all_edges.push_back(edge);
    while (getline(infile, line)) {
        istringstream iss(line);
        string token;
        // 解析三个字段
        vector<int> values;
        try {
            for (int i = 0; getline(iss, token, ','); i++) {
                values.push_back(stoi(token));
                if (i >= 2) break; // 只读取前三列
            }
        } catch (const exception& e) {
            cerr << "警告：第 " << line_num << " 行格式错误（已跳过）" << endl;
            continue;
        }
        if (values.size() != 3) {
            cerr << "警告：第 " << line_num << " 行列数错误（已跳过）" << endl;
            continue;
        }
        int src = values[0];
        int dst = values[1];
        int time = values[2];
        if (src == dst) {
            //do not consider self cycles;
            continue;
        }
        line_num++;
        Edge out_edge;
        out_edge.node_id = dst;
        out_edge.time = time;
        out_edge.edge_id = line_num;
        out_graph[src].emplace_back(out_edge);

        Edge in_edge;
        in_edge.node_id = src;
        in_edge.time = time;
        in_edge.edge_id = line_num;
        in_graph[dst].emplace_back(in_edge);

        Edge_info edge;
        edge.src = src;
        edge.dst = dst;
        edge.time = time;
        edge.edge_id = line_num;
        all_edges.emplace_back(edge);
    }
    num_edges_ = line_num;
    infile.close();
};

void Graph::reduce_out_graph (int node) {
    if (out_graph[node].size() == 0) {
        for (long unsigned int j = 0; j < in_graph[node].size(); j++) {
        int next_node = in_graph[node][j].node_id;
        for (auto it = out_graph[next_node].begin(); it != out_graph[next_node].end();) {
            if (it->node_id == node) {
                out_graph[next_node].erase(it);
            } else {
                it++;
            }
        }  // delete the corresponding edge in out_graph;
        reduce_out_graph (next_node);
        }
        in_graph[node].clear();
    } else {
        return;
    }
};

void Graph::reduce_in_graph (int node) {
    if (in_graph[node].size() == 0) {
        for (long unsigned int j = 0; j < out_graph[node].size(); j++) {
        int next_node = out_graph[node][j].node_id;
        for (auto it = in_graph[next_node].begin(); it != in_graph[next_node].end();) {
            if (it->node_id == node) {
                in_graph[next_node].erase(it);
            } else {
                it++;
            }
        }
        reduce_in_graph(next_node);
        }
    } else {
        return;
    }
};

void Graph::reduce_graph () {
    for (int i = 0; i < num_vertices_; i++) {
        reduce_out_graph(i);
        reduce_in_graph(i);
    }
};

void Graph::edge_filter (){
    for (int i = 1; i < num_vertices_; i++){
        int out_degree = out_graph[i].size();
        int in_degree = in_graph[i].size(); 
        // cout << "node: " << i << endl;
        // cout << "out degree: " << out_degree << endl;
        // cout << "in degree: " << in_degree << endl;
        for (int j = 0; j < in_degree; j++){
            int edge_id = in_graph[i][j].edge_id;
            if (out_degree == 0) {
                all_edges[edge_id].next_edge_start = 0;
                all_edges[edge_id].next_edge_end = 0;
            } else {
                int time_1 = in_graph[i][j].time;
                if (j == 0){
                    int k = 0;
                    while (out_graph[i][k].time <= time_1 && k < out_degree){
                        k++;
                    }
                    all_edges[edge_id].next_edge_start = k;
                    while (out_graph[i][k].time <= time_1 + window_size && k < out_degree){
                        k++;
                    }
                    all_edges[edge_id].next_edge_end = k;
                }else {
                    int start_ = all_edges[in_graph[i][j - 1].edge_id].next_edge_start;
                    while (out_graph[i][start_].time <= time_1 && start_ < out_degree){
                        start_++;
                    }
                    all_edges[edge_id].next_edge_start = start_;
                    int end_ = all_edges[in_graph[i][j - 1].edge_id].next_edge_end;
                    while (out_graph[i][end_].time <= time_1 + window_size && end_ < out_degree){
                        end_++;
                    }
                    all_edges[edge_id].next_edge_end = end_;
                }
            }
            // cout << in_graph[i][j].node_id << " -> " << i << " at time: " << time_1 << endl;
            // cout << "next start: " << all_edges[edge_id].next_edge_start << endl;
            // cout << "next end: " << all_edges[edge_id].next_edge_end << endl;
        }
        for (int j = 0; j < out_degree; j++){
            int edge_id = out_graph[i][j].edge_id;
            if (in_degree == 0) {
                all_edges[edge_id].last_edge_start = 0;
                all_edges[edge_id].last_edge_end = 0;
            } else {
                int time_1 = out_graph[i][j].time;
                if (j == 0){
                    int k = 0;
                    while (in_graph[i][k].time + window_size < time_1 && k < in_degree){
                        k++;
                    }
                    all_edges[edge_id].last_edge_start = k;
                    while (in_graph[i][k].time < time_1 && k < in_degree){
                        k++;
                    }
                    all_edges[edge_id].last_edge_end = k;
                } else {
                    int start_ = all_edges[out_graph[i][j - 1].edge_id].last_edge_start;
                    while (in_graph[i][start_].time + window_size < time_1 && start_ < in_degree){
                        start_++;
                    }
                    all_edges[edge_id].last_edge_start = start_;
                    int end_ = all_edges[out_graph[i][j - 1].edge_id].last_edge_end;
                    while (in_graph[i][end_].time < time_1 && end_ < in_degree){
                        end_++;
                    }
                    all_edges[edge_id].last_edge_end = end_;
                }
            }
            // cout << i << " -> " << out_graph[i][j].node_id << " at time: " << time_1 << endl;
            // cout << "last start: " << all_edges[edge_id].last_edge_start << endl;
            // cout << "last end: " << all_edges[edge_id].last_edge_end << endl;
        }
    }
};

void Graph::load_graph (string graph_path){
    read_graph(graph_path);
    edge_filter();
};

void Graph::Test () {
    // Test read graph function;
    cout << "Edge Info: " << endl;
    for (long unsigned int i = 1; i < all_edges.size(); i++) {
        cout << i << "-th edge: " << endl;
        cout << all_edges[i].src << " -> " << all_edges[i].dst << " at time: " << all_edges[i].time << endl;
        cout << "edge id: " << all_edges[i].edge_id << endl;
        cout << "next edge info: " << endl;
        int dst = all_edges[i].dst;
        for (int j = all_edges[i].next_edge_start; j < all_edges[i].next_edge_end; j++) {
            cout << "-- " << dst << " -> " << out_graph[dst][j].node_id << " at time: " << out_graph[dst][j].time << endl;
        }
        cout << "last edge info: " << endl;
        int src = all_edges[i].src;
        for (int j = all_edges[i].last_edge_start; j < all_edges[i].last_edge_end; j++) {
            cout << "-- " << in_graph[src][j].node_id << " -> " << src << " at time: " << in_graph[src][j].time << endl;
        }
        cout << endl;
    }
}

void Graph::DFS (int start_edge, int cur_depth, int cur_edge, bool dir) {
    if (dir == true) {
        int cur_node = all_edges[cur_edge].dst;
        for (int i = all_edges[cur_edge].next_edge_start; i < all_edges[cur_edge].next_edge_end; i++) {
            int next_edge = out_graph[cur_node][i].edge_id;
            int next_time = out_graph[cur_node][i].time;
            int next_node = out_graph[cur_node][i].node_id;
            // cout << "Check at edge: " << all_edges[next_edge].src << " -> " 
            // << all_edges[next_edge].dst << " at " << all_edges[next_edge].time << endl;
            if (next_time - all_edges[start_edge].time > window_size) {
                break;
            }
            if (next_node == all_edges[start_edge].src) {
                cycle_number++;
                max_cycle_length = max (max_cycle_length, cur_depth);
                each_result[cur_depth]++;
                // string path = "";
                // for (int t = 0; t < cur_depth + 1; t++) {
                //     path += to_string(all_edges[partial_path[t]].src) + " -> (" 
                //     + to_string(all_edges[partial_path[t]].time) + ") -> ";
                // }
                // path += to_string(all_edges[next_edge].src) + " -> (" 
                // + to_string(all_edges[next_edge].time) + ") -> " 
                // + to_string(all_edges[next_edge].dst);
                // cycle_path.insert(path);
                continue;
            }
            if (visited[next_node]) {
                continue; //skip repeated node;
            }
            // cout << "Explore at edge: " << all_edges[next_edge].src << " -> " 
            // << all_edges[next_edge].dst << " at " << all_edges[next_edge].time << endl;
            partial_path[cur_depth + 1] = next_edge;
            visited[next_node] = true;
            DFS (start_edge, cur_depth + 1, next_edge, dir);
            visited[next_node] = false;
        }
    } else {
        return;
    }
}

void Graph::Filter (bool dir) {
    std::cout << "Start Algorithm Filter! " << endl;
    running_time.reset();
    visited = new bool[num_vertices_];
    std::fill(visited, visited + num_vertices_, false);
    each_result = new int[MAX_LENGTH];
    for (int i = 0; i < MAX_LENGTH; i++) {
      each_result[i] = 0;
    }
    if (dir == true) {
        for (int i = 1; i < num_vertices_; i++) {
            for (auto it = out_graph[i].begin(); it != out_graph[i].end(); it++) {
                int start_edge = it->edge_id;
                partial_path[0] = start_edge;
                visited[it->node_id] = true;
                // cout << "Start at edge: " << i << " -> " << it->node_id << " at " << it->time << endl;
                DFS (start_edge, 0, start_edge, dir);
                visited[it->node_id] = false;
            }
        }
    } 
}

void Graph::unblock (int node, int time, bool dir) {
    if (dir == true) {
        if (time > close_time[node]) {
            close_time[node] = time;
            if (block_set_stamp_node[node] != stamp) {
                return;
            }
            for (auto it = unblock_list[node].begin(); it != unblock_list[node].end();) {
                if (it->node_time < time) {
                    unblock (it->node_id, it->node_time, dir);
                    unblock_list[node].erase(it);
                } else {
                    it++;
                }
            }
        }
    }
}

bool Graph::DFS_block (int start_edge, int cur_depth, int cur_edge, bool dir) {
    if (dir == true) {
        int start_node = all_edges[start_edge].src;
        int cur_node = all_edges[cur_edge].dst;
        int cur_time = all_edges[cur_edge].time;
        int lastp = 0;
        stamp_record[cur_node] = start_node;
        close_time[cur_node] = cur_time;

        for (int i = all_edges[cur_edge].next_edge_start; i < all_edges[cur_edge].next_edge_end; i++) {
            int next_edge = out_graph[cur_node][i].edge_id;
            int next_time = out_graph[cur_node][i].time;
            int next_node = out_graph[cur_node][i].node_id;
            if (next_time - all_edges[start_edge].time > window_size) {
                break;
            }
            if (next_node == start_node) {
                cycle_number++;
                max_cycle_length = max (max_cycle_length, cur_depth);
                each_result[cur_depth]++;
                if (next_time > lastp) {
                    lastp = next_time;
                }

                // string path = "";
                // for (int t = 0; t < cur_depth + 1; t++) {
                //     path += to_string(all_edges[partial_path[t]].src) + " -> (" 
                //     + to_string(all_edges[partial_path[t]].time) + ") -> ";
                // }
                // path += to_string(all_edges[next_edge].src) + " -> (" 
                // + to_string(all_edges[next_edge].time) + ") -> " 
                // + to_string(all_edges[next_edge].dst);

                // if (cycle_path.find(path) == cycle_path.end()) {
                //     cycle_path.insert(path);
                //     if (cur_cycle_number != 0) {
                //         add_cycle_path_by_new.push_back(path);
                //     }
                // }

                continue;
            }
            bool pass = false;
            if (close_time[next_node] <= next_time && stamp_record[next_node] == start_node) {
                pass = false;
            } else {
                partial_path[cur_depth + 1] = next_edge;
                pass = DFS_block (start_edge, cur_depth + 1, next_edge, dir);
            }
            if (!pass) {
                if (block_set_stamp_node[next_node] != stamp) {
                    Block_node temp;
                    std::vector<Block_node> temp1;
                    unblock_list[next_node] = temp1;
                    temp.node_id = cur_node;
                    temp.node_time = next_time;
                    unblock_list[next_node].push_back(temp);
                    block_set_stamp_node[next_node] = stamp;
                } else {
                    bool temp = false;
                    for (auto it = unblock_list[next_node].begin(); it != unblock_list[next_node].end(); it++) {
                        if (it->node_id == cur_node && it->node_time == next_time) {
                            temp = true;
                        }
                    }
                    if (!temp) {
                        unblock_list[next_node].push_back({cur_node, next_time});
                    }
                }
            } else {
                if (next_time > lastp) {
                    lastp = next_time;
                }
            }
        }
        if (lastp > 0) {
            unblock(cur_node, lastp, dir);
        }
        return (lastp > 0);
    } else {
        return false;
    }
}

void Graph::Block (bool dir) {
    cout << "Start Algorithm Block!" << endl;
    running_time.reset();
    cycle_number = 0;
    stamp_record.resize(num_vertices_, 0);
    close_time.resize(num_vertices_, MAX_NUMBER);
    unblock_list.resize(num_vertices_);
    block_set_stamp_node.resize(num_vertices_, 0);
    each_result = new int[MAX_LENGTH];
    for (int i = 0; i < MAX_LENGTH; i++) {
        each_result[i] = 0;
    }
    stamp = 1;
    if (dir == true) {
        for (int i = 1; i < num_vertices_; i++) {
            for (auto it = out_graph[i].rbegin(); it != out_graph[i].rend(); it++){
                int start_edge = it->edge_id;
                stamp++;
                partial_path[0] = start_edge;
                DFS_block (start_edge, 0, start_edge, dir);
            }
        }
    }
    // cout << "End Block!" << endl;
}

void Graph::update_single_edge (int src, int dst, int time){
    if (src == dst) {
        //do not consider self cycles;
        return;
    }
    //step 1: insert information to out, in graph and all_edges:
    num_edges_++; //the number of edges plus 1;
    Edge out_edge;
    out_edge.node_id = dst;
    out_edge.time = time;
    out_edge.edge_id = num_edges_;
    out_graph[src].emplace_back(out_edge);
    Edge in_edge;
    in_edge.node_id = src;
    in_edge.time = time;
    in_edge.edge_id = num_edges_;
    in_graph[dst].emplace_back(in_edge);
    Edge_info edge;
    edge.src = src;
    edge.dst = dst;
    edge.time = time;
    edge.edge_id = num_edges_;
    all_edges.emplace_back(edge);
    // cout << "Step 1 finish!" << endl;

    //step 2: record the last edge start and end of this edge;
    int in_degree = in_graph[src].size();
    int out_degree = out_graph[src].size();

    if (in_degree == 0) {
        all_edges[num_edges_].last_edge_start = 0;
        all_edges[num_edges_].last_edge_end = 0;
    } else {
        if (out_degree > 1){
            int start_ = all_edges[out_graph[src][out_degree - 2].edge_id].last_edge_start;
            while (in_graph[src][start_].time + window_size < time && start_ < in_degree) {
                start_++;
            }
            all_edges[num_edges_].last_edge_start = start_;
            int end_ = all_edges[out_graph[src][out_degree - 2].edge_id].last_edge_end;
            while (in_graph[src][end_].time < time && end_ < in_degree) {
                end_++;
            }
            all_edges[num_edges_].last_edge_end = end_;
        } else {
            int k = 0;
            while (in_graph[src][k].time + window_size < time && k < in_degree) {
                k++;
            }
            all_edges[num_edges_].last_edge_start = k;
            while (in_graph[src][k].time < time && k < in_degree) {
                k++;
            }
            all_edges[num_edges_].last_edge_end = k;
        }
    }
    // cout << "Step 2 finish!" << endl;
    
    //step 3: record the end place of those edges which can arrive at src;
    for (int i = all_edges[num_edges_].last_edge_start; i < all_edges[num_edges_].last_edge_end; i++) {
        int edge_id = in_graph[src][i].edge_id;
        if (all_edges[edge_id].next_edge_start == all_edges[edge_id].next_edge_end) {
            int start_ = all_edges[edge_id].next_edge_start;
            while (out_graph[src][start_].time < all_edges[edge_id].time + 1 && start_ < out_degree) {
                start_++;
            }
            all_edges[edge_id].next_edge_start = start_;
        }
        all_edges[edge_id].next_edge_end = out_degree;
    }
    // cout << "Step 3 finish!" << endl;

    //step 4: init the next edge start and end of this edge;
    all_edges[num_edges_].next_edge_start = out_graph[dst].size();
    all_edges[num_edges_].next_edge_end = out_graph[dst].size();

    // cout << "Edge Info: " << endl;
    // int i = num_edges_;
    // cout << i << "-th edge: " << endl;
    // cout << all_edges[i].src << " -> " << all_edges[i].dst << " at time: " << all_edges[i].time << endl;
    // cout << "edge id: " << all_edges[i].edge_id << endl;
    // cout << "last edge info: " << endl;
    // for (int j = all_edges[i].last_edge_start; j < all_edges[i].last_edge_end; j++) {
    //     cout << "- " << in_graph[src][j].node_id << " -> " << src << " at time: " << in_graph[src][j].time << endl;
    //     cout << "this edge's next edge: " << endl;
    //     for (int k = all_edges[in_graph[src][j].edge_id].next_edge_start; k < all_edges[in_graph[src][j].edge_id].next_edge_end; k++) {
    //         cout << "-- " << src << " -> " << out_graph[src][k].node_id << " at time: " << out_graph[src][k].time << endl;
    //     }
    // }
    // cout << endl;

};

bool Graph::DFS_block_dynamic (int start_edge, int cur_depth, int cur_edge, bool break_point, int break_place) {
    if (break_point == true) {
        int start_node = all_edges[start_edge].src;
        int cur_node = all_edges[cur_edge].dst;
        int cur_time = all_edges[cur_edge].time;
        int lastp = 0;
        stamp_record[cur_node] = stamp;
        close_time[cur_node] = cur_time;

        for (int i = all_edges[cur_edge].next_edge_start; i < all_edges[cur_edge].next_edge_end; i++) {
            int next_edge = out_graph[cur_node][i].edge_id;
            int next_time = out_graph[cur_node][i].time;
            int next_node = out_graph[cur_node][i].node_id;

            if (next_time - all_edges[start_edge].time > window_size) {
                break;
            }

            if (next_node == start_node && next_time < all_edges[start_edge].time) {
                add_cycle_number++;
                max_cycle_length = max (max_cycle_length, cur_depth);
                each_result[cur_depth]++;
                if (next_time > lastp) {
                    lastp = next_time;
                }

                // string path = "";
                // for (int t = break_place; t < cur_depth + 1; t++){
                //     path += to_string(all_edges[partial_path[t]].src) + " -> ("
                //      + to_string(all_edges[partial_path[t]].time) + ") -> ";
                // }
                // path += to_string(all_edges[next_edge].src) + " -> (" 
                // + to_string(all_edges[next_edge].time) + ") -> " ;
                // for (int t = 0; t < break_place; t++) {
                //     path += to_string(all_edges[partial_path[t]].src) + " -> (" 
                //     + to_string(all_edges[partial_path[t]].time) + ") -> " ;
                // }
                // path += to_string (all_edges[partial_path[break_place]].src);
                // add_cycle_path_by_dynamic.push_back(path);

                continue;
            }

            bool pass = false;

            if (visited[next_node] || (close_time[next_node] <= next_time && stamp_record[next_node] == stamp)) {
                pass = false;
            } else {
                partial_path[cur_depth + 1] = next_edge;
                visited[next_node] = true;
                pass = DFS_block_dynamic (start_edge, cur_depth + 1, next_edge, break_point, break_place);
                visited[next_node] = false;
            }
            if (!pass) {
                if (block_set_stamp_node[next_node] != stamp) {
                    Block_node temp;
                    std::vector<Block_node> temp1;
                    unblock_list[next_node] = temp1;
                    temp.node_id = cur_node;
                    temp.node_time = next_time;
                    unblock_list[next_node].push_back(temp);
                    block_set_stamp_node[next_node] = stamp;
                } else {
                    bool temp = false;
                    for (auto it = unblock_list[next_node].begin(); it != unblock_list[next_node].end(); it++) {
                        if (it->node_id == cur_node && it->node_time == next_time) {
                            temp = true;
                        }
                    }
                    if (!temp) {
                        unblock_list[next_node].push_back({cur_node, next_time});
                    }
                }
            } else {
                if (next_time > lastp) {
                    lastp = next_time;
                }
            }
        }
        if (lastp > 0) {
            unblock(cur_node, lastp, true);
        }
        return (lastp > 0);
    } else {
        int cur_node = all_edges[cur_edge].dst;
        int start_node = all_edges[start_edge].src;
        int cur_time = all_edges[cur_edge].time;
        int start_time = all_edges[start_edge].time;
        int lastp = 0;
        stamp_record[cur_node] = stamp;
        close_time[cur_node] = cur_time;

        for (auto it = out_graph[cur_node].rbegin(); it != out_graph[cur_node].rend(); it++) {
            int next_node = it->node_id;
            int next_time = it->time;
            int next_edge = it->edge_id;
            if (next_time < start_time - window_size) {
                break;
            }

            if (next_time == cur_time) {
                continue;
            }

            if (next_node == start_node) {
                add_cycle_number++;
                max_cycle_length = max (max_cycle_length, cur_depth);
                each_result[cur_depth]++;
                if (next_time > lastp) {
                    lastp = next_time;
                }

                // string path = "";
                // for (int t = 0; t < cur_depth + 1; t++) {
                //     path += to_string(all_edges[partial_path[t]].src) + " -> (" 
                //     + to_string(all_edges[partial_path[t]].time) + ") -> ";
                // }
                // path += to_string(all_edges[next_edge].src) + " -> (" 
                // + to_string(all_edges[next_edge].time) + ") -> " 
                // + to_string(all_edges[next_edge].dst);
                // add_cycle_path_by_dynamic.push_back(path);

                continue;
            }

            bool pass = false;
            if (close_time[next_node] <= next_time && stamp_record[next_node] == stamp) {
                pass = false;
            } else {
                partial_path[cur_depth + 1] = next_edge;
                visited[next_node] = true;
                if (next_time < cur_time) {
                    pass = DFS_block_dynamic (start_edge, cur_depth + 1, next_edge, true, cur_depth + 1);
                } else if (next_time > cur_time) {
                    pass = DFS_block_dynamic (start_edge, cur_depth + 1, next_edge, false, cur_depth + 1);
                }
                visited[next_node] = false;
            }
            if (!pass) {
                if (block_set_stamp_node[next_node] != stamp) {
                    Block_node temp;
                    std::vector<Block_node> temp1;
                    unblock_list[next_node] = temp1;
                    temp.node_id = cur_node;
                    temp.node_time = next_time;
                    unblock_list[next_node].push_back(temp);
                    block_set_stamp_node[next_node] = stamp;
                } else {
                    bool temp = false;
                    for (auto it = unblock_list[next_node].begin(); it != unblock_list[next_node].end(); it++) {
                        if (it->node_id == cur_node && it->node_time == next_time) {
                            temp = true;
                        }
                    }
                    if (!temp) {
                        unblock_list[next_node].push_back({cur_node, next_time});
                    }
                }
            } else {
                if (next_time > lastp) {
                    lastp = next_time;
                }
            }
        }
        if (lastp > 0) {
            unblock(cur_node, lastp, true);
        }
        return (lastp > 0);
    }
}

void Graph::Dynamic_search (int start_edge) {
    add_cycle_number = 0;
    stamp++;
    partial_path[0] = start_edge;
    visited[all_edges[start_edge].dst] = true;
    DFS_block_dynamic (start_edge, 0, start_edge, false, 0);
    visited[all_edges[start_edge].dst] = false;
}

void Graph::load_update_edges(string update_file) {
    ifstream infile;
    infile.open(update_file.c_str());
    if (!infile.is_open()) {
        cerr << "错误：无法打开输入文件 " << update_file << endl;
        return;
    }
    out_graph.resize(num_vertices_);
    in_graph.resize(num_vertices_);
    // read cur graph before update;
    string line;
    int line_num = 0;
    int read_num = 0;
    Edge_info edge;
    all_edges.push_back(edge);
    while (getline(infile, line)) {
        read_num++;
        istringstream iss(line);
        string token;
        // 解析三个字段
        vector<int> values;
        try {
            for (int i = 0; getline(iss, token, ','); i++) {
                values.push_back(stoi(token));
                if (i >= 2) break; // 只读取前三列
            }
        } catch (const exception& e) {
            cerr << "警告：第 " << line_num << " 行格式错误（已跳过）" << endl;
            continue;
        }
        if (values.size() != 3) {
            cerr << "警告：第 " << line_num << " 行列数错误（已跳过）" << endl;
            continue;
        }
        int src = values[0];
        int dst = values[1];
        int time = values[2];
        if (src == dst) {
            //do not consider self cycles;
            continue;
        }

        line_num++;
        Edge out_edge;
        out_edge.node_id = dst;
        out_edge.time = time;
        out_edge.edge_id = line_num;
        out_graph[src].emplace_back(out_edge);

        Edge in_edge;
        in_edge.node_id = src;
        in_edge.time = time;
        in_edge.edge_id = line_num;
        in_graph[dst].emplace_back(in_edge);

        Edge_info edge;
        edge.src = src;
        edge.dst = dst;
        edge.time = time;
        edge.edge_id = line_num;
        all_edges.emplace_back(edge);
        if (read_num >= update_edges_place) {
            break;
        }
    }
    num_edges_ = line_num;
    edge_filter();
    //do update graph;
    //do block to record the cur_cycle;
    stamp_record.resize(num_vertices_, 0);
    close_time.resize(num_vertices_, MAX_NUMBER);
    unblock_list.resize(num_vertices_);
    block_set_stamp_node.resize(num_vertices_, 0);
    visited = new bool[num_vertices_];
    std::fill(visited, visited + num_vertices_, false);
    each_result = new int[MAX_LENGTH];
    for (int i = 0; i < MAX_LENGTH; i++) {
        each_result[i] = 0;
    }

    for (int i = 1; i < num_vertices_; i++) {
        for (auto it = out_graph[i].rbegin(); it != out_graph[i].rend(); it++){
            int start_edge = it->edge_id;
            stamp++;
            partial_path[0] = start_edge;
            DFS_block (start_edge, 0, start_edge, true);
        }
    }
    cur_cycle_number = cycle_number;
    //load the update edges:

    while (getline (infile, line)) {
        istringstream iss(line);
        string token;
        vector<int> values;
        try {
            for (int i = 0; getline(iss, token, ','); i++) {
                values.push_back(stoi(token));
                if (i >= 2) break; // 只读取前三列
            }
        } catch (const exception& e) {
            cerr << "警告: 格式错误（已跳过）" << endl;
            continue;
        }
        int src = values[0];
        int dst = values[1];
        int time = values[2];
        if (src == dst) {
            continue;
        }
        update_edge temp;
        temp.src = src;
        temp.dst = dst;
        temp.time = time;
        update_edge_list.emplace_back(temp);
    }
    infile.close();
}

void Graph::update(string update_file) {
    string load_file = "/root/CycleEnum/Data/" + update_file + ".txt";
    load_update_edges(load_file);
    ofstream outfile;
    string update_result = "/root/CycleEnum/Result/Update/Update_time.txt";
    outfile.open(update_result.c_str(), ios::app);
    if (!outfile.is_open()) {
        cerr << "错误：无法打开输出文件 " << update_result << endl;
        return;
    }
    outfile << "[Dataset] " << update_file << endl;
    outfile << "[Window] " << window_size << endl;

    ModuleTimer dynamic_time;
    ModuleTimer naive_time;

    //load the update edges:
    int update_edge_number = update_edge_list.size();
    // cout << update_edge_number << endl;
    for (int i = 0; i < update_edge_number; i++) {
        int src = update_edge_list[i].src;
        int dst = update_edge_list[i].dst;
        int time = update_edge_list[i].time;

        // outfile << i << "-th Update edge: " << src << " -> " << dst << " at " << time << endl;
        //update data;
        update_single_edge (src, dst, time);
        //do dynamic search;
        // dynamic_search_cost.reset();
        dynamic_time.start();
        Dynamic_search (num_edges_);
        dynamic_time.stop();
        // outfile << "[Dynamic Cost] " << dynamic_search_cost.elapsedMicroseconds() << endl;
        // outfile << "Add Cycle Number by Dynamic Search: " << add_cycle_number << endl;
        // for (auto x : add_cycle_path_by_dynamic) {
        //     outfile << x << endl;
        // }
        // add_cycle_path_by_dynamic.clear();
    }
    outfile << "[Dynamic] " << std::fixed << std::setprecision(2) << (dynamic_time.total_time() / (double) dynamic_time.count()) << endl;
    outfile << endl;
    outfile.close();
}

void Graph::update_naive(string update_file) {
    string load_file = "/root/CycleEnum/Data/" + update_file + ".txt";
    load_update_edges(load_file);
    ofstream outfile;
    string update_result = "/root/CycleEnum/Result/Update/Update_naive_time.txt";
    outfile.open(update_result.c_str(), ios::app);
    if (!outfile.is_open()) {
        cerr << "错误：无法打开输出文件 " << update_result << endl;
        return;
    }
    outfile << "[Dataset] " << update_file << endl;
    outfile << "[Window] " << window_size << endl;

    ModuleTimer naive_time;

    //load the update edges:
    int update_edge_number = update_edge_list.size();
    // cout << update_edge_number << endl;
    for (int i = 0; i < update_edge_number; i++) {
        int src = update_edge_list[i].src;
        int dst = update_edge_list[i].dst;
        int time = update_edge_list[i].time;

        // outfile << i << "-th Update edge: " << src << " -> " << dst << " at " << time << endl;
        //update data;
        update_single_edge (src, dst, time);
        //naive update:
        naive_time.start();
        for (int i = 1; i < num_vertices_; i++) {
            for (auto it = out_graph[i].rbegin(); it != out_graph[i].rend(); it++){
                int start_edge = it->edge_id;
                if (all_edges[start_edge].time < time - window_size) {
                    break;
                }
                stamp++;
                partial_path[0] = start_edge;
                DFS_block (start_edge, 0, start_edge, true);
            }
        }
        naive_time.stop();
        // outfile << "[Naive Cost] " << new_search_cost.elapsedMicroseconds() << endl;
        // outfile << "Add Cycle Number by New Search: " << add_cycle_path_by_new.size() << endl;
        // for (auto x : add_cycle_path_by_new) {
        //     outfile << x << endl;
        // }
        // add_cycle_path_by_new.clear();
    }
    outfile << "[Naive] " << std::fixed << std::setprecision(2) << (naive_time.total_time() / (double) naive_time.count()) << endl;
    outfile << endl;
    outfile.close();
}

void Graph::Throughput_and_tail(string update_file) {
    string load_file = "/root/CycleEnum/Data/" + update_file + ".txt";
    load_update_edges(load_file);
    ofstream outfile;
    string update_result = "/root/CycleEnum/Result/Update/Throughput_and_Tail.txt";
    outfile.open(update_result.c_str(), ios::app);
    if (!outfile.is_open()) {
        cerr << "错误：无法打开输出文件 " << update_result << endl;
        return;
    }
    outfile << "[Dataset] " << update_file << endl;
    outfile << "[Window] " << window_size << endl;

    int update_edge_number = update_edge_list.size();
    constexpr std::chrono::milliseconds window_duration(50);  // 10ms窗口 
    std::vector<double> latencies;                     // 记录每次更新的延迟
    latencies.reserve(update_edge_number);             // 预分配内存避免动态扩容开销
    uint64_t throughput_counter = 0;                   // 当前窗口内的任务计数
    auto window_start = std::chrono::steady_clock::now();
    ModuleTimer throughput;
    for (int i = 0; i < update_edge_number; i++) {
        int src = update_edge_list[i].src;
        int dst = update_edge_list[i].dst;
        int time = update_edge_list[i].time;

        auto task_start = std::chrono::steady_clock::now();
        //update data;
        update_single_edge (src, dst, time);
        //do dynamic search;
        throughput.start();
        Dynamic_search (num_edges_);
        throughput.stop();

       // 记录任务结束时间和延迟
       auto task_end = std::chrono::steady_clock::now();
       double latency_ms = std::chrono::duration<double, std::milli>(task_end - task_start).count();
       latencies.push_back(latency_ms);
       throughput_counter++;
       
       // 检查时间窗口是否结束
       auto current_time = std::chrono::steady_clock::now();
       if (current_time - window_start >= window_duration) {
           // 输出当前窗口的吞吐量
           std::cout << "[Throughput] " << throughput_counter / 50 << " ops/ms\n";
           outfile << "[Throughput] " << throughput_counter / 50 << " ops/ms\n";
           throughput_counter = 0;
           window_start = current_time; // 重置窗口起始时间
       }
    }
    outfile << "[All Cost] " << throughput.total_time() << " us\n";
    outfile << "[All Tasks] " << throughput.count() << "\n";
    cout << "[All Throughput] " << std::fixed << std::setprecision(2) << (throughput.total_time() / (double) throughput.count()) << "ops/us\n";
    outfile << "[All Throughput] " << std::fixed << std::setprecision(2) << (throughput.total_time() / (double) throughput.count()) << "ops/us\n";

    if (!latencies.empty()) {
        std::sort(latencies.begin(), latencies.end());
        size_t total = latencies.size();
        size_t p99_idx = static_cast<size_t>(total * 0.99);
        size_t p999_idx = static_cast<size_t>(total * 0.999);
        std::cout << "[Tail Latency] P99: " << latencies[p99_idx] << "ms, "
                  << "P99.9: " << latencies[p999_idx] << "ms\n";
        outfile << "[Tail Latency] P99: " << latencies[p99_idx] << "ms, "
                  << "P99.9: " << latencies[p999_idx] << "ms\n";
    }
    outfile << endl;
    outfile.close();
}

void Graph::Path_Result_Output (string path_file) {
    std::ofstream ofs_path (path_file.c_str(), ios::out); 
    for (auto x : cycle_path) {
        ofs_path << x << endl;
        // cout << x << endl;
    }
    ofs_path.close();
}

