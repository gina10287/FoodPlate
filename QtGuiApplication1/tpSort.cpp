#include <iostream>
#include <vector>
#include <queue>

#include "tpSort.h"

using namespace std;

//topological sort with Kahn's algorithm

vector<int> tmpp;

void topo::init(int num) {
	_adjList.clear();
	_adjListW.clear();
	_count.clear();
	_num = num;
	for (int i = 0; i < _num; i++) {
		vector<int> tmp;
		vector<double> tmpd;
		_adjList.push_back(tmp);
		_adjListW.push_back(tmpd);
		_count.push_back(0);
	}
}

void topo::addEdge(int start, int end, double weight)
{
	_adjList[start].push_back(end);
	_adjListW[start].push_back(weight);
}

void topo::delEdge(int start, int end, double weight) {
	/*bool findElement = false;
	for (int j = 0; j < _adjList[start].size(); j++) {
		if (_adjList[start][j] == end) {
			findElement = true;
			_adjList[start].erase(_adjList[start].begin() + j);
			_adjListW[start].erase(_adjListW[start].begin() + j);
			break;
		}

	}
	if (!findElement) {
		cout << "Element not found in myvector\n";
	}
	*/
	vector<int>::iterator it = find(_adjList[start].begin(), _adjList[start].end(), end);
	vector<double>::iterator itw = find(_adjListW[start].begin(), _adjListW[start].end(), weight);
	if (it != _adjList[start].end())
	{
		_adjList[start].erase(it);
		//cout << "Element found in myvector: " << *it << '\n';
	}
	else
	{
		cout << "Element not found in myvector\n";
	}
	
}

void topo::printAdjList()
{
	for (int i = 0; i < _adjList.size(); i++)
	{
		cout << i << " :";
		for (int j = 0; j < _adjList[i].size(); j++)
		{
			cout << " " << _adjList[i][j];
		}
		cout << endl;
	}
}

void topo::topoSort() {
	for (int i = 0; i < _count.size(); i++)
		_count[i] = 0;

	// indegree for each node
	for (int i = 0; i < _adjList.size(); i++)
		for (int j = 0; j < _adjList[i].size(); j++)
			_count[_adjList[i][j]]++;

	queue<int> Q;
	for (int i = 0; i < _count.size(); i++)
		if (_count[i] == 0)
			Q.push(i);

	for (int i = 0; i < _num; i++) {
		if (Q.empty())
			break;

		int s = Q.front();
		Q.pop();
		sortResult.push_back(s);

		_count[s] = -1;

		for (int j = 0; j < _adjList[s].size(); j++) {
			int t = _adjList[s][j];
			_count[t]--;
			if (_count[t] == 0)
				Q.push(t);
		}
	}
	for (int i = 0; i < _num; i++)
		_count[i] = 0;
}

// Returns true if the graph contains a cycle, else false.
void topo::isCyclic()
{
	tmpp.clear();
	_flag = false;
	//-1 not explore, 0 been explore, 1 fully explored
	//vector<int> visited;
	_visited.clear();
	for (int i = 0; i < _num; i++)
		_visited.push_back(-1);

	for (int i = 0; i < _num; i++)
	{
		dfs(i);
		if (_flag)
			break;


		_visited.clear();
		for (int i = 0; i < _num; i++)
			_visited.push_back(-1);
	}


	vector<int>::iterator cycleTest;

	if (_flag == true)
	{
		cycleTest = find(tmpp.begin(), tmpp.end(), _cycEnd);
		tmpp.erase(tmpp.begin(), cycleTest);
	}


	//for (int i = 0; i < tmpp.size(); i++)
	//	cout << "tmp size: " << tmpp[i] << endl;

	//del the min weight edge
	vector<double> tmpWeight;
	if (_flag == true)
	{
		//cout << "find cycle, del the min edge in the cycle" << endl;
		for (int i = 0; i < tmpp.size(); i++)
			tmpWeight.push_back(getWeight(tmpp[i], tmpp[(i + 1) % tmpp.size()]));

		double minWeight = *min_element(tmpWeight.begin(), tmpWeight.end());

		int idx;

		for (int i = 0; i < tmpWeight.size(); i++)
			if (tmpWeight[i] == minWeight)
				idx = i;

		delEdge(tmpp[idx], tmpp[(idx + 1) % tmpp.size()], minWeight);

	}


}

void topo::dfs(int s)
{
	//cout << s << endl;
	tmpp.push_back(s);
	_visited[s] = 0;
	for (int i = 0; i < _adjList[s].size(); i++)
	{
		if (_visited[_adjList[s][i]] == -1)
			dfs(_adjList[s][i]);
		else if (_visited[_adjList[s][i]] == 0)
		{
			_flag = true;
			_cycEnd = _adjList[s][i];
		}
		if (_flag)
			break;

	}
	_visited[s] = 1;
	if (!_flag)
		tmpp.pop_back();
}

double topo::getWeight(int start, int end)
{
	int index = -1;
	for (int i = 0; i < _adjList[start].size(); i++)
		if (_adjList[start][i] == end)
			index = i;

	return _adjListW[start][index];
}