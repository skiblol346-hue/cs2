#pragma once

#include <cstddef>
#include <cstdint>

class c_econ_item_view;

namespace econ_item_attribute_manager {
	struct attribute_value_t {
		uint16_t def_index;
		float value;
	};

	void create(c_econ_item_view* item, int paint_kit, float wear, int seed);
	void create(c_econ_item_view* item, int paint_kit, float wear, int seed, const attribute_value_t* extra_attributes, size_t extra_count);
	bool get_value(c_econ_item_view* item, uint16_t def_index, float& value);
	void remove(c_econ_item_view* item);
}
