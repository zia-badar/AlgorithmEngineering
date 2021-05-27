#ifndef P3S_BUCKET_H
#define P3S_BUCKET_H

class p3s_bucket
{
 public:
	set<p3> p3s_uvw_sorted;
	set<p3> p3s_average_weight_sorted;
	set<p3> p3s_uvw_sorted_reset;
	set<p3> p3s_average_weight_sorted_reset;

 public:
	void add_p3(int u, int v, int w, float weight_vu, float weight_vw, float weight_uw)
	{
		if (search_by_uvw(u, v, w))
			int i = 1 / 0;

		float average_weight = (weight_vu + weight_vw + abs(weight_uw)) / 3.0;

		while (search_by_weight(u, v, w, average_weight))
			average_weight += 0.1;

		p3s_uvw_sorted.insert(p3(u, v, w, average_weight, p3::UVW_SEARCHABLE));
		p3s_average_weight_sorted.insert(p3(u, v, w, average_weight, p3::WEIGHT_SEARCHABLE));
	}

	void remove_p3_by_uvw(int u, int v, int w)
	{
		if (!search_by_uvw(u, v, w))
			int i = 1 / 0;
		float weight = retrieve_by_uvw(u, v, w).weight;
		p3s_uvw_sorted.erase(p3(u, v, w, weight, p3::UVW_SEARCHABLE));
		p3s_average_weight_sorted.erase(p3(u, v, w, weight, p3::WEIGHT_SEARCHABLE));
	}

	bool search_by_uvw(int u, int v, int w)
	{
		return p3s_uvw_sorted.find(p3(u, v, w, 0, p3::UVW_SEARCHABLE)) != p3s_uvw_sorted.end();
	}

	bool search_by_weight(int u, int v, int w, float weight)
	{
		return p3s_average_weight_sorted.find(p3(u, v, w, weight, p3::WEIGHT_SEARCHABLE))
			!= p3s_average_weight_sorted.end();
	}

	p3 retrieve_by_uvw(int u, int v, int w)
	{
		if (!search_by_uvw(u, v, w))
			int i = 1 / 0;
		return *(p3s_uvw_sorted.find(p3(u, v, w, 0, p3::UVW_SEARCHABLE)));
	}

	p3 retrieve_max_weight_p3()
	{
		if (is_empty())
			int i = 1 / 0;
		return *(p3s_average_weight_sorted.begin());
	}

	void save_bucket_state_for_reset()
	{
		p3s_uvw_sorted_reset = p3s_uvw_sorted;
		p3s_average_weight_sorted_reset = p3s_average_weight_sorted;
	}

	void reset()
	{
		p3s_uvw_sorted = p3s_uvw_sorted_reset;
		p3s_average_weight_sorted = p3s_average_weight_sorted_reset;
	}

	set<p3> const* get_all_p3s_weight_sorted()
	{
		return &p3s_average_weight_sorted;
	}

	int get_p3s_count()
	{
		return p3s_uvw_sorted.size();
	}

	bool is_empty()
	{
		return p3s_uvw_sorted.empty();
	}
};
#endif