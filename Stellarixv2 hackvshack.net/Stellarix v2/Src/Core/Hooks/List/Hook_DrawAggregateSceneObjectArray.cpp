#include "Hook_DrawAggregateSceneObjectArray.hpp"
#include <Core/Interfaces/List/IMaterialSystem.hpp>
#include <Core/Utils/Logger/Logger.hpp>


auto __fastcall Hook_DrawAggregateSceneObjectArray( void* a1, void* a2, CBaseSceneData* pSceneData ) -> void*
{
	return DrawAggregateSceneObjectArray_o.unsafe_fastcall<void*>( a1, a2, pSceneData );
}
