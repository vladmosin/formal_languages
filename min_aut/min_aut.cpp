/*
input structure:
	size_of_alphabet (for example, 4)
	alphabet (for example, a 54 lfkg 09 =)
	number_of_states (for example, 3)
	description for all states (for example, 0 2 // 0 - not terminal, 2 number of edges
											 2 a // 2 - number of state with which state 0 is connected
												 // a - edge's letter
											 2 =
											 
											 1 1
											 0 54
											 
											 0 0)
	start state (for example, 1)
*/



#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

using std::vector;
using std::string;
using std::unordered_map;
using std::pair;

void input_data(std::ifstream& in, vector<string>& alphabet,
				vector<unordered_map<string, int>>& edges,
				vector<int>& terminal_states,
				int& start_vertex, int& num_of_vertexes) {

	int size_of_alphabet;
	in >> size_of_alphabet;
	alphabet.resize(size_of_alphabet);
	for (int i = 0; i < size_of_alphabet; i++) {
		in >> alphabet[i];
	}

	in >> num_of_vertexes;
	terminal_states.resize(num_of_vertexes);
	edges.resize(num_of_vertexes);
	fill(terminal_states.begin(), terminal_states.end(), 0);
	int is_terminal; // 0 - not terminal, 1 - is terminal
	int num_of_edges, end_of_edge;
	string letter;
	for (int i = 0; i < num_of_vertexes; i++) {
		in >> is_terminal;
		if (is_terminal)
			terminal_states[i] = 1;
		in >> num_of_edges;
		for (int j = 0; j < num_of_edges; j++) {
			in >> end_of_edge >> letter;
			edges[i][letter] = end_of_edge;
		}
	}
	in >> start_vertex;
}

void dfs(int vertex, vector<unordered_map<string, int>>& edges, vector<int>& visited) {
	visited[vertex] = 1;
	for (auto neighbour : edges[vertex]) {
		if (!visited[neighbour.second])
			dfs(neighbour.second, edges, visited);
	}
}

void build_graph(vector<vector<int>>& graph, vector<unordered_map<string, int>>& edges,
				 vector<string> alphabet) {

	int size = edges.size();
	graph.resize(size * size);
	for (int r = 0; r < size; r++) {
		for (int s = r; s < size; s++) {
			for (string letter : alphabet) {
				auto iter_r = edges[r].find(letter);
				auto iter_s = edges[s].find(letter);
				if (iter_s != edges[s].end() && iter_r != edges[r].end()) {
					graph[std::min(iter_s->second, iter_r->second) * size + 
								   std::max(iter_s->second, iter_r->second)].push_back(r * size + s);
				}
			}
		}
	}
}

void dfs_in_new_graph(int vertex, vector<vector<int>>& graph,
					  vector<vector<int>>& visited, int size) {

	visited[vertex / size][vertex % size] = 1;
	visited[vertex % size][vertex / size] = 1;

	for (int next_vertex : graph[vertex]) {
		if (!visited[next_vertex % size][next_vertex / size])
			dfs_in_new_graph(next_vertex, graph, visited, size);
	}
}

void component_dfs(int vertex, vector<vector<int>>& visited,
				   vector<int>& vertex_component_number, int component_number,
				   vector<int>& reachable) {
	vertex_component_number[vertex] = component_number;
	for (std::size_t i = 0; i < visited[vertex].size(); i++) {
		if (!visited[vertex][i] && !vertex_component_number[i] && reachable[i]) {
			component_dfs(i, visited, vertex_component_number, component_number, reachable);
		}
	}
}

int gen_components(vector<int>& vertex_component_number, vector<vector<int>>& visited,
					vector<int>& reachable) {
	int size = visited.size(), component_number = 1;
	vertex_component_number.resize(size);
	fill(vertex_component_number.begin(), vertex_component_number.end(), 0);
	for (int i = 0; i < size; i++) {
		if (!vertex_component_number[i] && reachable[i])
			component_dfs(i, visited, vertex_component_number, component_number++, reachable);
	}

	return --component_number;
}

void edges_in_final_graph(vector<unordered_map<string, int>>& old_edges,
						  vector<unordered_map<string, int>>& new_edges,
						  vector<int>& vertex_component_number, int size,
						  vector<int>& terminal_state_components,
						  vector<int>& terminal_states) {

	new_edges.resize(size + 1);
	terminal_state_components.resize(size + 1);
	fill(terminal_state_components.begin(), terminal_state_components.end(), 0);
	for (std::size_t i = 0; i < terminal_states.size(); i++) {
		terminal_state_components[vertex_component_number[i]] = terminal_states[i];
	}

	for (std::size_t i = 0; i < old_edges.size(); i++) {
		for (const pair<string, int>& x : old_edges[i]) {
			new_edges[vertex_component_number[i]][x.first] = vertex_component_number[x.second];
		}
	}
}

void output_graph(std::ofstream& out,
		         vector<unordered_map<string, int>>& edges,
				 vector<int>& terminal_state_components,
				 int start_component) {
	out << "digraph G {\n";
	for (std::size_t i = 1; i < edges.size(); i++) {
		for (const pair<string, int>& x : edges[i]) {
			out << "	" << i << " -> " << x.second << "[label=\"" << x.first << "\"];\n";
		}
	}

	if (terminal_state_components[start_component])
		out << "	" << start_component << " [peripheries=2,label=\"start\"];\n";
	else
		out << "	" << start_component << " [label=\"start\"];\n";
	for (std::size_t i = 1; i < edges.size(); i++) {
		if (terminal_state_components[i] && (int)i != start_component)
			out << "	" << i << " [peripheries=2];\n";
	}
	out << "}";
}

int main(int argc, char* argv[]) {
	std::ifstream in(argv[1]);
	std::ofstream out("min_aut.txt");

	vector<string> alphabet;
	vector<unordered_map<string, int>> edges;
	vector<int> terminal_states;
	int start_vertex;
	int num_of_vertexes;
	
	input_data(in, alphabet, edges, terminal_states, start_vertex, num_of_vertexes);

	vector<int> reachable(num_of_vertexes);
	fill(reachable.begin(), reachable.end(), 0);

	dfs(start_vertex, edges, reachable);

	vector<vector<int>> graph;
	build_graph(graph, edges, alphabet);

	vector<int> vertex_component_number;
	vector<vector<int>> is_different;
	is_different.resize(num_of_vertexes);

	for (int i = 0; i < num_of_vertexes; i++) {
		is_different[i].resize(num_of_vertexes);
		fill(is_different[i].begin(), is_different[i].end(), 0);
	}

	for (int i = 0; i < num_of_vertexes; i++) {
		for (int j = i + 1; j < num_of_vertexes; j++) {
			if ((terminal_states[i] != terminal_states[j]) && !is_different[i][j])
				dfs_in_new_graph(i * num_of_vertexes + j, graph, is_different, num_of_vertexes);
		}
	}
	vector<int> terminal_state_components;
	int start_component;
	int number_of_components = gen_components(vertex_component_number, is_different, reachable);

	vector<unordered_map<string, int>> new_edges;

	edges_in_final_graph(edges, new_edges, vertex_component_number,
						 number_of_components, terminal_state_components,
						 terminal_states);

	start_component = vertex_component_number[start_vertex];

	output_graph(out, new_edges, terminal_state_components, start_component);

    return 0;
}


