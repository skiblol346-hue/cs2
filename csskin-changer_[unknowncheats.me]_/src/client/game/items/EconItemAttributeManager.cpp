#include "game/items/EconItemAttributeManager.hpp"
#include "game/entities/c_cs_player_pawn.hpp"
#include "game/interfaces/i_mem_alloc.hpp"

namespace {
	struct econ_item_attribute_t {
		char     pad_0000[0x30];
		uint16_t def_index;
		char     pad_0032[2];
		float    value;
		float    init_value;
		int32_t  refundable_currency;
		bool     set_bonus;
		char     pad_0041[7];
	};

	struct ptr_game_vector_t {
		uint64_t  size;
		uintptr_t ptr;
	};

	enum : uint16_t {
		ATTR_PAINT   = 6,
		ATTR_PATTERN = 7,
		ATTR_WEAR    = 8,
	};
}

namespace econ_item_attribute_manager {

	void create(c_econ_item_view* item, int paint_kit, float wear, int seed) {
		create(item, paint_kit, wear, seed, nullptr, 0);
	}

	void create(c_econ_item_view* item, int paint_kit, float wear, int seed, const attribute_value_t* extra_attributes, size_t extra_count) {
		if (!item || (paint_kit <= 0 && extra_count == 0) || (extra_count > 0 && !extra_attributes))
			return;

		auto* attr_list = item->m_attribute_list();
		if (!attr_list)
			return;

		auto* vec = reinterpret_cast<ptr_game_vector_t*>(&attr_list->m_attributes());

		const size_t base_count = paint_kit > 0 ? 3 : 0;
		const size_t attr_count = base_count + extra_count;
		auto* attrs = reinterpret_cast<econ_item_attribute_t*>(vec->ptr);
		bool reuse = attrs && vec->size == attr_count;
		if (reuse) {
			if (paint_kit > 0)
				reuse = attrs[0].def_index == ATTR_PAINT
					&& attrs[1].def_index == ATTR_PATTERN
					&& attrs[2].def_index == ATTR_WEAR;
			for (size_t i = 0; reuse && i < extra_count; ++i)
				reuse = attrs[base_count + i].def_index == extra_attributes[i].def_index;
		}

		if (!reuse) {
			if (attrs)
				GameFree(attrs);
			attrs = static_cast<econ_item_attribute_t*>(GameAlloc(attr_count * sizeof(econ_item_attribute_t)));
			if (!attrs) {
				vec->size = 0;
				vec->ptr = 0;
				return;
			}
			memset(attrs, 0, attr_count * sizeof(econ_item_attribute_t));
		}

		if (paint_kit > 0) {
			attrs[0].def_index = ATTR_PAINT;
			attrs[0].value = static_cast<float>(paint_kit);
			attrs[0].init_value = attrs[0].value;

			attrs[1].def_index = ATTR_PATTERN;
			attrs[1].value = static_cast<float>(seed >= 0 ? seed : 0);
			attrs[1].init_value = attrs[1].value;

			attrs[2].def_index = ATTR_WEAR;
			attrs[2].value = wear >= 0.0f ? wear : 0.01f;
			attrs[2].init_value = attrs[2].value;
		}

		for (size_t i = 0; i < extra_count; ++i) {
			auto& attribute = attrs[base_count + i];
			attribute.def_index = extra_attributes[i].def_index;
			attribute.value = extra_attributes[i].value;
			attribute.init_value = attribute.value;
		}

		vec->size = attr_count;
		vec->ptr = reinterpret_cast<uintptr_t>(attrs);
	}

	bool get_value(c_econ_item_view* item, uint16_t def_index, float& value) {
		if (!item)
			return false;

		auto* attr_list = item->m_attribute_list();
		if (!attr_list)
			return false;

		auto* vec = reinterpret_cast<ptr_game_vector_t*>(&attr_list->m_attributes());
		auto* attrs = reinterpret_cast<econ_item_attribute_t*>(vec->ptr);
		if (!attrs || vec->size > 256)
			return false;

		for (size_t i = 0; i < vec->size; ++i) {
			if (attrs[i].def_index == def_index) {
				value = attrs[i].value;
				return true;
			}
		}
		return false;
	}

	void remove(c_econ_item_view* item) {
		if (!item)
			return;

		auto* attr_list = item->m_attribute_list();
		if (!attr_list)
			return;

		auto* vec = reinterpret_cast<ptr_game_vector_t*>(&attr_list->m_attributes());
		if (vec->size == 0)
			return;

		void* ptr = reinterpret_cast<void*>(vec->ptr);
		vec->size = 0;
		vec->ptr  = 0;

		GameFree(ptr);
	}
}
