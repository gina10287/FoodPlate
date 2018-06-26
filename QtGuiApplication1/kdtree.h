#ifndef SUNMOON_DYNEMIC_KD_TREE
#define SUNMOON_DYNEMIC_KD_TREE
#include<algorithm>
#include<vector>
template<typename T, size_t kd>
class kd_tree{
public:
	struct point{
		T d[kd];
		inline T dist(const point &x)const{
			T ret = 0;
			for (size_t i = 0; i<kd; ++i)ret += std::abs(d[i] - x.d[i]);
			return ret;
		}
		inline bool operator<(const point &b)const{
			return d[0]<b.d[0];
		}
		inline point operator-(const point &b)const{
			point tmp;
			T dist = 0;
			for (size_t i = 0; i < kd; ++i)
			{
				tmp.d[i] = d[i] - b.d[i];
				dist += tmp.d[i] * tmp.d[i];
			}
			dist = sqrt(dist);
			for (size_t i = 0; i < kd; ++i)
				tmp.d[i] /= dist;
			return tmp;
		}
		inline T dot(const point &b)const{
			T ret = 0;
			for (size_t i = 0; i<kd; ++i)ret += (d[i] * x.d[i]);
			return ret;
		}
	};
private:
	struct node{
		node *l, *r;
		point pid;
		node(const point &p) :l(0), r(0), pid(p){}
	}*root;
	const T INF;
	std::vector<node*> A;
	int s;
	struct __cmp{
		int sort_id;
		inline bool operator()(const node*x, const node*y)const{
			return x->pid.d[sort_id]<y->pid.d[sort_id];
		}
	}cmp;
	void clear(node *o){
		if (!o)return;
		clear(o->l);
		clear(o->r);
		delete o;
	}
	node* build(int k, int l, int r){
		if (l>r)return 0;
		if (k == kd)k = 0;
		int mid = (l + r) / 2;
		cmp.sort_id = k;
		std::nth_element(A.begin() + l, A.begin() + mid, A.begin() + r + 1, cmp);
		node *ret = A[mid];
		ret->l = build(k + 1, l, mid - 1);
		ret->r = build(k + 1, mid + 1, r);
		return ret;
	}
	inline int heuristic(const T h[])const{
		int ret = 0;
		for (size_t i = 0; i<kd; ++i)ret += h[i];
		return ret;
	}
	node **mnp;
	int mnk;
	void findmin(node*&o, int d, int k){
		if (!o)return;
		if (!mnp || o->pid.d[d]<(*mnp)->pid.d[d]){
			mnp = &o;
			mnk = k;
		}
		findmin(o->l, d, (k + 1) % kd);
		if (d == k)return;
		findmin(o->r, d, (k + 1) % kd);
	}
	void nearest_for_erase(node *&u, int k, const point &x, T *h, T &mndist){
		if (u == 0 || heuristic(h) >= mndist)return;
		point now = u->pid;
		int dist = u->pid.dist(x), old = h[k];


		if (dist<mndist){
			mnp = &u;
			mnk = k;
			if (!(mndist = dist))return;
		}
		if (x.d[k]<u->pid.d[k]){
			nearest_for_erase(u->l, (k + 1) % kd, x, h, mndist);
			h[k] = abs(x.d[k] - u->pid.d[k]);
			nearest_for_erase(u->r, (k + 1) % kd, x, h, mndist);
		}
		else{
			nearest_for_erase(u->r, (k + 1) % kd, x, h, mndist);
			h[k] = abs(x.d[k] - u->pid.d[k]);
			nearest_for_erase(u->l, (k + 1) % kd, x, h, mndist);
		}
		h[k] = old;
	}
	void nearest_for_erase_alpha(node *&u, int k, const point &x, T *h, T &mndist, vector<double>& nor){
		if (u == 0 || heuristic(h) >= mndist)return;
		point now = u->pid;
		int dist = u->pid.dist(x), old = h[k];

		point norF = u->pid - x;

		double value = dist*(1-(norF.d[0]*nor[0]+norF.d[1]*nor[1]));

		if (value<mndist){
			mnp = &u;
			mnk = k;
			if (!(mndist = value))return;
		}
		if (x.d[k]<u->pid.d[k]){
			nearest_for_erase(u->l, (k + 1) % kd, x, h, mndist);
			h[k] = abs(x.d[k] - u->pid.d[k]);
			nearest_for_erase(u->r, (k + 1) % kd, x, h, mndist);
		}
		else{
			nearest_for_erase(u->r, (k + 1) % kd, x, h, mndist);
			h[k] = abs(x.d[k] - u->pid.d[k]);
			nearest_for_erase(u->l, (k + 1) % kd, x, h, mndist);
		}
		h[k] = old;
	}
public:
	kd_tree(const T &INF) :INF(INF), s(0){}
	inline void clear(){
		clear(root), root = 0;
	}
	inline void build(int n, const point *p){
		clear(root), A.resize(s = n);
		for (int i = 0; i<n; ++i)A[i] = new node(p[i]);
		root = build(0, 0, n - 1);
	}
	inline bool erase(point p){
		T mndist = 1, h[kd] = {};
		nearest_for_erase(root, 0, p, h, mndist);
		if (mndist)return 0;
		for (node **o = mnp;;){
			if ((*o)->r);
			else if ((*o)->l){
				(*o)->r = (*o)->l;
				(*o)->l = 0;
			}
			else{
				delete *o;
				(*o) = 0;
				--s;
				return 1;
			}
			mnp = 0;
			findmin((*o)->r, mnk, (mnk + 1) % kd);
			(*o)->pid = (*mnp)->pid;
			o = mnp;
		}
	}
	inline T nearest(const point &x){
		T mndist = INF, h[kd] = {};
		nearest_for_erase(root, 0, x, h, mndist);
		return mndist;
	}
	inline T nearestAlpha(const point &x, vector<double> nor, point & rt){
		T mndist = INF, h[kd] = {};
		nearest_for_erase_alpha(root, 0, x, h, mndist, nor);
		rt = root->pid;
		return mndist;
	}
	inline int size(){ return s; }
};
#endif