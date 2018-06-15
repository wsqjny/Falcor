/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#include "Framework.h"
#include "ConstantBuffer.h"
#include "Graphics/Program/ProgramVersion.h"
#include "Buffer.h"
#include "glm/glm.hpp"
#include "Texture.h"
#include "Graphics/Program/ProgramReflection.h"
#include "API/Device.h"

#include "Renderer.h"
#include "utils/Gui.h"



namespace Falcor
{
    ConstantBuffer::~ConstantBuffer() = default;

    ConstantBuffer::ConstantBuffer(const std::string& name, const ReflectionResourceType::SharedConstPtr& pReflectionType, size_t size) :
        VariablesBuffer(name, pReflectionType, size, 1, Buffer::BindFlags::Constant, Buffer::CpuAccess::Write)
    {
    }

    ConstantBuffer::SharedPtr ConstantBuffer::create(const std::string& name, const ReflectionResourceType::SharedConstPtr& pReflectionType, size_t overrideSize)
    {
        size_t size = (overrideSize == 0) ? pReflectionType->getSize() : overrideSize;
        SharedPtr pBuffer = SharedPtr(new ConstantBuffer(name, pReflectionType, size));
        return pBuffer;
    }

    ConstantBuffer::SharedPtr ConstantBuffer::create(Program::SharedPtr& pProgram, const std::string& name, size_t overrideSize)
    {
        const auto& pProgReflector = pProgram->getReflector();
        const auto& pParamBlockReflection = pProgReflector->getDefaultParameterBlock();
        ReflectionVar::SharedConstPtr pBufferReflector = pParamBlockReflection ? pParamBlockReflection->getResource(name) : nullptr;

        if (pBufferReflector)
        {
            ReflectionResourceType::SharedConstPtr pResType = pBufferReflector->getType()->asResourceType()->inherit_shared_from_this::shared_from_this();
            if(pResType && pResType->getType() == ReflectionResourceType::Type::ConstantBuffer)
            {
                return create(name, pResType, overrideSize);
            }
        }
        logError("Can't find a constant buffer named \"" + name + "\" in the program");
        return nullptr;
    }

    bool ConstantBuffer::uploadToGPU(size_t offset, size_t size)
    {
        if (mDirty) mpCbv = nullptr;
        return VariablesBuffer::uploadToGPU(offset, size);
    }

    ConstantBufferView::SharedPtr ConstantBuffer::getCbv() const
    {
        if (mpCbv == nullptr)
        {
            mpCbv = ConstantBufferView::create(Resource::shared_from_this());
        }
        return mpCbv;
    }

	const std::string val_to_string(ReflectionBasicType::Type type, const uint8_t* data)
	{
		std::string asString("{ ");
		size_t offset = 0;

#define val_2_string_from_type_(typeName, baseType) \
			case typeName: \
			asString.append(std::to_string( *reinterpret_cast<const baseType*>(data + offset) )); \
			offset += sizeof(baseType)

#define val_2_string_from_type(typeName, baseType, endString) \
			val_2_string_from_type_(typeName, baseType); \
			asString.append(endString)

		switch (type)
		{
			val_2_string_from_type(ReflectionBasicType::Type::Bool4, bool, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Bool3, bool, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Bool2, bool, " , ");
			val_2_string_from_type_(ReflectionBasicType::Type::Bool, bool);
			break;
			val_2_string_from_type(ReflectionBasicType::Type::Uint4, uint32_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Uint3, uint32_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Uint2, uint32_t, " , ");
			val_2_string_from_type_(ReflectionBasicType::Type::Uint, uint32_t);
			break;
			val_2_string_from_type(ReflectionBasicType::Type::Uint64_4, uint64_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Uint64_3, uint64_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Uint64_2, uint64_t, " , ");
			val_2_string_from_type_(ReflectionBasicType::Type::Uint64, uint64_t);
			break;
			val_2_string_from_type(ReflectionBasicType::Type::Int4, int32_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Int3, int32_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Int2, int32_t, " , ");
			val_2_string_from_type_(ReflectionBasicType::Type::Int, int32_t);
			break;
			val_2_string_from_type(ReflectionBasicType::Type::Int64_4, int64_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Int64_3, int64_t, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Int64_2, int64_t, " , ");
			val_2_string_from_type_(ReflectionBasicType::Type::Int64, int64_t);
			break;
			val_2_string_from_type(ReflectionBasicType::Type::Float4, float, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Float3, float, " , ");
			val_2_string_from_type(ReflectionBasicType::Type::Float2, float, " , ");
			val_2_string_from_type_(ReflectionBasicType::Type::Float, float);
			break;
		case ReflectionBasicType::Type::Float2x2:
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data + sizeof(float) * 2));
			break;
		case ReflectionBasicType::Type::Float2x3:
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data + sizeof(float) * 3));
			break;
		case ReflectionBasicType::Type::Float2x4:
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data + sizeof(float) * 4));
			break;
		case ReflectionBasicType::Type::Float3x2:
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data + sizeof(float) * 2)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data + sizeof(float) * 4));
			break;
		case ReflectionBasicType::Type::Float3x3:
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data + sizeof(float) * 3)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data + sizeof(float) * 6));
			break;
		case ReflectionBasicType::Type::Float3x4:
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data + sizeof(float) * 4)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data + sizeof(float) * 8));
			break;
		case ReflectionBasicType::Type::Float4x2:
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data + sizeof(float) * 2)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data + sizeof(float) * 4)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float2, data + sizeof(float) * 6));
			break;
		case ReflectionBasicType::Type::Float4x3:
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data + sizeof(float) * 3)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data + sizeof(float) * 6)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float3, data + sizeof(float) * 9));
			break;
		case ReflectionBasicType::Type::Float4x4:
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data + sizeof(float) * 4)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data + sizeof(float) * 8)).append(",\n");
			asString.append(val_to_string(ReflectionBasicType::Type::Float4, data + sizeof(float) * 12));
			break;
		case ReflectionBasicType::Type::Unknown:
		default:
			break;
		}
#undef val_2_string_from_type
#undef val_2_string_from_type_

		asString.append(" }");
		return asString;
	}

	bool ConstantBuffer::guiWidgetForData(Gui* pGui, ReflectionBasicType::Type type, uint8_t* data, const std::string& name)
	{
		std::string displayName = name;
		displayName.resize(name.size() + 3, 0);
		size_t offset = 0;
		unsigned displayIndex = 0;
		bool returnValue = false;


#define update_array_index_name() displayName[displayName.size() - 3] = '['; \
		displayName[displayName.size() - 2] = '0' + displayIndex; \
		displayName[displayName.size() - 1] = ']'; \
		displayIndex++
#define concatStrings_(a, b) a##b
#define concatStrings(a, b) concatStrings_(a, b)
#define to_gui_widget(widgetName, baseType) \
		returnValue |= pGui-> concatStrings(add, widgetName)(displayName.c_str(), *reinterpret_cast<baseType*>(data + offset)); \
		offset += sizeof(baseType);
		
		switch (type)
		{
		case ReflectionBasicType::Type::Bool4:
			update_array_index_name();
			to_gui_widget(CheckBox, bool);
		case ReflectionBasicType::Type::Bool3:
			update_array_index_name();
			to_gui_widget(CheckBox, bool);
		case ReflectionBasicType::Type::Bool2:
			update_array_index_name();
			to_gui_widget(CheckBox, bool);
			update_array_index_name();
		case ReflectionBasicType::Type::Bool:
			to_gui_widget(CheckBox, bool);
			break;
		case ReflectionBasicType::Type::Uint4:
		case ReflectionBasicType::Type::Uint64_4:
		case ReflectionBasicType::Type::Int4:
		case ReflectionBasicType::Type::Int64_4:
			update_array_index_name();
			to_gui_widget(IntVar, int);
		case ReflectionBasicType::Type::Uint3:
		case ReflectionBasicType::Type::Uint64_3:
		case ReflectionBasicType::Type::Int3:
		case ReflectionBasicType::Type::Int64_3:
			update_array_index_name();
			to_gui_widget(IntVar, int);
		case ReflectionBasicType::Type::Uint2:
		case ReflectionBasicType::Type::Uint64_2:
		case ReflectionBasicType::Type::Int2:
		case ReflectionBasicType::Type::Int64_2:
			update_array_index_name();
			to_gui_widget(IntVar, int);
			update_array_index_name();
		case ReflectionBasicType::Type::Uint:
		case ReflectionBasicType::Type::Uint64:
		case ReflectionBasicType::Type::Int:
		case ReflectionBasicType::Type::Int64:
			to_gui_widget(IntVar, int);
			break;
		case ReflectionBasicType::Type::Float:
			returnValue |= pGui->addFloatVar(name.c_str(), *reinterpret_cast<float*>(data));
			break;
		case ReflectionBasicType::Type::Float2:
			returnValue |= pGui->addFloat2Var(name.c_str(), *reinterpret_cast<glm::vec2*>(data));
			break;
		case ReflectionBasicType::Type::Float3:
			returnValue |= pGui->addFloat3Var(name.c_str(), *reinterpret_cast<glm::vec3*>(data));
			break;
		case ReflectionBasicType::Type::Float4:
			returnValue |= pGui->addFloat4Var(name.c_str(), *reinterpret_cast<glm::vec4*>(data));
			break;
		case ReflectionBasicType::Type::Float2x2:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data + sizeof(float) * 2, displayName);
			break;
		case ReflectionBasicType::Type::Float2x3:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float3, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float3, data + sizeof(float) * 3, displayName);
			break;
		case ReflectionBasicType::Type::Float2x4:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 4, displayName);
			break;
		case ReflectionBasicType::Type::Float3x2:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data + sizeof(float) * 2, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data + sizeof(float) * 4, displayName);
			break;
		case ReflectionBasicType::Type::Float3x3:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float3, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float3, data + sizeof(float) * 3, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float3, data + sizeof(float) * 6, displayName);
			break;
		case ReflectionBasicType::Type::Float3x4:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 4, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 8, displayName);
			break;
		case ReflectionBasicType::Type::Float4x2:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data + sizeof(float) * 2, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data + sizeof(float) * 4, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float2, data + sizeof(float) * 6, displayName);
			break;
		case ReflectionBasicType::Type::Float4x3:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data, name);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 3, name);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 6, name);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 9, name);
			break;
		case ReflectionBasicType::Type::Float4x4:
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 4, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 8, displayName);
			update_array_index_name();
			returnValue |= guiWidgetForData(pGui, ReflectionBasicType::Type::Float4, data + sizeof(float) * 12, displayName);
			break;
		case ReflectionBasicType::Type::Unknown:
			break;
		default:
			break;
		}
#undef to_gui_widget
#undef concatStrings
#undef concatStrings_
#undef update_array_index_name

		return returnValue;
	}

	void ConstantBuffer::onGuiRenderMemberInternal(Gui* pGui, const std::string& memberName, size_t memberOffset, size_t memberSize, const std::string& memberTypeString, const ReflectionBasicType::Type& memberType, float textSpacing)
	{
		// Display reflection data and gather offset
		pGui->addText("Name: ", false, textSpacing);
		pGui->addText(memberName.c_str(), true, textSpacing + 50.0f);
		pGui->addText("Offset: ", false, textSpacing);
		pGui->addText(std::to_string(memberOffset).c_str(), true, textSpacing + 50.0f);
		pGui->addText("	Size: ", true, textSpacing + 150.0f);
		pGui->addText(std::to_string(memberSize).c_str(), true, textSpacing + 200.0f);
		pGui->addText("	Type: ", true, textSpacing + 250.0f);
		pGui->addText(memberTypeString.c_str(), true, textSpacing + 300.0f);

		// Display data from the stage memory
		pGui->addText((val_to_string(memberType, mData.data() + memberOffset)).c_str());
		VariablesBuffer::mDirty |= guiWidgetForData(pGui, memberType, mData.data() + memberOffset, memberName);

		pGui->addSeparator();
	}

	void ConstantBuffer::onGuiRenderInternal(Gui* pGui, const ReflectionStructType* pStruct, const std::string& currentStructName, size_t startOffset, float textSpacing)
	{
		static std::unordered_map<std::string, int32> guiArrayIndices;

		auto memberIt = pStruct->begin();
		while (memberIt != pStruct->end())
		{
			size_t numMembers = 1;
			size_t memberSize = 0;
			ReflectionBasicType::Type memberType = ReflectionBasicType::Type::Unknown;
			std::string memberName = (*memberIt)->getName();
			constexpr float kTextSpacingOffset = 50.0f;
			const ReflectionBasicType* pBasicType = (*memberIt)->getType()->asBasicType();
			ReflectionArrayType* pArrayType = nullptr;
			bool baseTypeIsStruct = false;
			bool arrayGroupStatus = false;
			size_t currentOffset = startOffset + (*memberIt)->getOffset();

			// First test is not basic type
			if (!pBasicType)
			{
				// recurse through struct if possible
				auto* pStructType = (*memberIt)->getType()->asStructType();
				if (pStructType)
				{
					// Iterate through the internal struct
					if (pGui->beginGroup(memberName))
					{
						memberName.push_back('.');
						onGuiRenderInternal(pGui, pStructType, memberName, currentOffset, textSpacing + kTextSpacingOffset);
						memberName.pop_back();

						pGui->endGroup();
					}
					pGui->addSeparator();

					// skip to next member
					++memberIt;
					continue;
				}

				// if array type gather info for iterating through elements
				pArrayType = const_cast<ReflectionArrayType*>((*memberIt)->getType()->asArrayType());
				if (pArrayType)
				{
					pGui->addSeparator();

					// only iterate through array if it is displaying
					arrayGroupStatus = pGui->beginGroup(memberName + "[]");
					if (!arrayGroupStatus)
					{
						++memberIt;
						pGui->addSeparator();
						continue;
					}

					auto* elementBasicType = pArrayType->getType()->asBasicType();
					numMembers = pArrayType->getArraySize();
					memberSize = pArrayType->getArrayStride();

					if (elementBasicType)
					{ 
						memberType = elementBasicType->getType();
					}
					else
					{
						// for special case of array of structures
						baseTypeIsStruct = true;
					}

					textSpacing += kTextSpacingOffset;
				}
				else if (!pStructType)
				{
					// Other types could be presented here
					return;
				}
			}
			else
			{
				// information if only basic type
				memberType = pBasicType->getType();
				memberSize = pBasicType->getSize();
			}

			
			// Walks through all members in the array
			// for (unsigned i = 0; i < numMembers; ++i)
			unsigned memberIndex = 0;
			
			{
				std::string displayName = memberName;
				if (numMembers > 1)
				{
					// display information for specific index of array
					int32& refGuiArrayIndex = guiArrayIndices[currentStructName + displayName];
					pGui->addIntVar((std::string("Index (Size : ") + std::to_string(numMembers) + ") ").c_str(), refGuiArrayIndex, 0, static_cast<int>(numMembers) - 1);
					memberIndex = refGuiArrayIndex;
					currentOffset += (memberSize * memberIndex);
					displayName.append("[").append(std::to_string(memberIndex)).append("]");
				}

				if (baseTypeIsStruct)
				{
					// For arrays of structs, display dropdown for struct before recursing through struct members
					if (pGui->beginGroup(displayName))
					{
						displayName.push_back('.');
						onGuiRenderInternal(pGui, pArrayType->getType()->asStructType(), displayName, currentOffset, textSpacing + kTextSpacingOffset);
						pGui->endGroup();
					}
				}
				else
				{
					// for basic types
					onGuiRenderMemberInternal(pGui, displayName, currentOffset, memberSize, to_string(memberType), memberType, textSpacing);
				}
				
				currentOffset += memberSize;
			}

			if (arrayGroupStatus)
			{
				pGui->endGroup();
			}

			++memberIt;
		}
	}

	void ConstantBuffer::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
	{
		auto* pStruct = mpReflector->asResourceType()->getStructType()->asStructType();

		if (pGui->beginGroup(std::string("ConstantBuffer").append(mName)))
		{
			pGui->addSeparator();

			// begin recursion on first struct
			onGuiRenderInternal(pGui, pStruct, "", 0);

			// dirty flag for uploading will be set by GUI
			uploadToGPU();

			pGui->endGroup();
		}
	}
}