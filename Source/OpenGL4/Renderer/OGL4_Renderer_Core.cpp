#include "OGL4_Renderer_Core.h"
#include "OGL4_GPUContentManager.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace OpenGL4 {

	void OpenGL4_Renderer::Bind_Framebuffer(const GFX_API::Framebuffer* FB) {
		
		glBindFramebuffer(GL_FRAMEBUFFER, *(unsigned int*)FB->GL_ID);
		for (unsigned int i = 0; i < FB->BOUND_RTs.size(); i++) {
			auto RT = FB->BOUND_RTs[i];
			if (RT.RT_READTYPE == GFX_API::RT_READSTATE::CLEAR) {
				glClearColor(RT.CLEAR_COLOR.x, RT.CLEAR_COLOR.y, RT.CLEAR_COLOR.z, 1);
				if (RT.ATTACHMENT_TYPE == GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_COLOR0) {
					glClear(GL_COLOR_BUFFER_BIT);
				}
				else if (RT.ATTACHMENT_TYPE == GFX_API::RT_ATTACHMENTs::TEXTURE_ATTACHMENT_DEPTH) {
					glClear(GL_DEPTH_BUFFER_BIT);
				}
			}
		}
	}
	void Bind_Uniform(const unsigned int& PROGRAM_ID, const GFX_API::Material_Uniform* uniform);
	void OpenGL4_Renderer::Bind_MatInstance(GFX_API::Material_Instance* MATINST) {
		unsigned int PROGRAM_GLID = *(unsigned int*)GFXContentManager->Find_GFXShaderProgram_byID(MATINST->Material_Type)->GL_ID;
		glUseProgram(PROGRAM_GLID);
		//Bind uniforms!
		for (GFX_API::Material_Uniform UNIFORM : MATINST->UNIFORM_LIST) {
			Bind_Uniform(PROGRAM_GLID, &UNIFORM);
		}
		//Bind uniform textures!
		for (GFX_API::Texture_Access TEXTURE : MATINST->TEXTURE_LIST) {
			if (TEXTURE.ACCESS_TYPE == GFX_API::TEXTURE_ACCESS::SAMPLER_OPERATION && TEXTURE.TEXTURE_ID) {
				glActiveTexture(GL_TEXTURE0 + TEXTURE.BINDING_POINT);
				GFX_API::GFX_Texture* GFXTEXTURE = GFXContentManager->Find_GFXTexture_byID(TEXTURE.TEXTURE_ID);
				if (GFXTEXTURE) {
					glBindTexture(GL_TEXTURE_2D, *(unsigned int*)GFXTEXTURE->GL_ID);
				}
			}
		}
		//Bind image textures!
		for (GFX_API::Texture_Access TEXTURE : MATINST->TEXTURE_LIST) {
			if (TEXTURE.ACCESS_TYPE == GFX_API::TEXTURE_ACCESS::IMAGE_OPERATION && TEXTURE.TEXTURE_ID) {
				unsigned int TEXTURE_GLID = *(unsigned int*)GFXContentManager->Find_GFXTexture_byID(TEXTURE.TEXTURE_ID)->GL_ID;
				glBindImageTexture(TEXTURE.BINDING_POINT, TEXTURE_GLID, 0, GL_FALSE, 0, Find_OGLOperationType(TEXTURE.OP_TYPE), Find_ImageTexture_InternalFormat(TEXTURE.CHANNELs));
			}
		}
	}
	void OpenGL4_Renderer::Set_DepthTest(GFX_API::DEPTH_MODEs MODE, GFX_API::DEPTH_TESTs TEST) {
		GFX->Check_Errors();
		//Set Depth Func at start!
		switch (MODE) {
		case GFX_API::DEPTH_MODEs::DEPTH_OFF:
			cout << "Depth Test is closed!\n";
			glDisable(GL_DEPTH_TEST);
			return;
		case GFX_API::DEPTH_MODEs::DEPTH_READ_WRITE:
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(Find_OGLDepthTest(TEST));
			return;
		case GFX_API::DEPTH_MODEs::DEPTH_READ_ONLY:
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
			glDepthFunc(Find_OGLDepthTest(TEST));
			return;
		default:
			LOG_ERROR("Intended Depth Buffer mode isn't supported in OGL4_Renderer::Set_DepthTest()!");
			return;
		}
	}
	void OpenGL4_Renderer::Set_CullingMode(GFX_API::CULL_MODE MODE) {
		GFX->Check_Errors();
		switch (MODE) {
		case GFX_API::CULL_MODE::CULL_OFF:
			glDisable(GL_CULL_FACE);
			GFX->Check_Errors();
			return;
		case GFX_API::CULL_MODE::CULL_FRONT:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			GFX->Check_Errors();
			return;
		case GFX_API::CULL_MODE::CULL_BACK:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			GFX->Check_Errors();
			return;
		default:
			LOG_ERROR("Intended Culling Mode isn't supported by Set_CullingMode!");
			return;
		}
	}
	void OpenGL4_Renderer::Set_LineWidth(float WIDTH) {
		LINEWIDTH = WIDTH;
		glLineWidth(WIDTH);
	}
	void OpenGL4_Renderer::DrawTriangle(const GFX_API::GFX_Mesh* MESH) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		OGL4_MESH* GL_MESH = (OGL4_MESH*)MESH->GL_ID;
		if (GL_MESH != nullptr) {
			glBindVertexArray(GL_MESH->VAO);
		}
		//Indexed Rendering
		if (MESH->INDEX_COUNT) {
			glDrawElements(GL_TRIANGLES, MESH->INDEX_COUNT, GL_UNSIGNED_INT, 0);
		}
		//Non-Indexed Rendering
		else {
			glDrawArrays(GL_TRIANGLES, 0, MESH->VERTEX_COUNT);
		}
	}
	void OpenGL4_Renderer::DrawPoint(const GFX_API::GFX_Point* POINTBUFFER) {
		glPointSize(10.0f);
		OGL4_MESH* GL_MESH = (OGL4_MESH*)POINTBUFFER->GL_ID;
		GFX->Check_Errors();
		glBindVertexArray(GL_MESH->VAO);
		GFX->Check_Errors();
		glDrawArrays(GL_POINTS, 0, POINTBUFFER->POINT_COUNT);
		GFX->Check_Errors();
	}
	void OpenGL4_Renderer::DrawLine(const GFX_API::GFX_Point* POINTBUFFER) {
		OGL4_MESH* GL_MESH = (OGL4_MESH*)POINTBUFFER->GL_ID;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(GL_MESH->VAO);
		glDrawArrays(GL_LINE_STRIP, 0, POINTBUFFER->POINT_COUNT);
	}		
	void OpenGL4_Renderer::DrawSpecialShader(GFX_API::Material_Instance* MATINST, unsigned int VERTEXCOUNT, const std::vector<GFX_API::Material_Uniform>& OverridenUniforms) {
		unsigned int PROGRAM_GLID = *(unsigned int*)GFXContentManager->Find_GFXShaderProgram_byID(MATINST->Material_Type)->GL_ID;
		glUseProgram(PROGRAM_GLID);
		//Bind previous uniforms!
		for (GFX_API::Material_Uniform UNIFORM : MATINST->UNIFORM_LIST) {
			Bind_Uniform(PROGRAM_GLID, &UNIFORM);
		}
		//Now override the uniforms!
		for (GFX_API::Material_Uniform UNIFORM : OverridenUniforms) {
			Bind_Uniform(PROGRAM_GLID, &UNIFORM);
		}
		//Bind uniform textures!
		for (GFX_API::Texture_Access TEXTURE : MATINST->TEXTURE_LIST) {
			if (TEXTURE.ACCESS_TYPE == GFX_API::TEXTURE_ACCESS::SAMPLER_OPERATION && TEXTURE.TEXTURE_ID) {
				glActiveTexture(GL_TEXTURE0 + TEXTURE.BINDING_POINT);
				GFX_API::GFX_Texture* GFXTEXTURE = GFXContentManager->Find_GFXTexture_byID(TEXTURE.TEXTURE_ID);
				if (GFXTEXTURE) {
					glBindTexture(GL_TEXTURE_2D, *(unsigned int*)GFXTEXTURE->GL_ID);
				}
			}
		}
		//Bind image textures!
		for (GFX_API::Texture_Access TEXTURE : MATINST->TEXTURE_LIST) {
			if (TEXTURE.ACCESS_TYPE == GFX_API::TEXTURE_ACCESS::IMAGE_OPERATION && TEXTURE.TEXTURE_ID) {
				unsigned int TEXTURE_GLID = *(unsigned int*)GFXContentManager->Find_GFXTexture_byID(TEXTURE.TEXTURE_ID)->GL_ID;
				glBindImageTexture(TEXTURE.BINDING_POINT, TEXTURE_GLID, 0, GL_FALSE, 0, Find_OGLOperationType(TEXTURE.OP_TYPE), Find_ImageTexture_InternalFormat(TEXTURE.CHANNELs));
			}
		}


		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_POINTS, 0, VERTEXCOUNT);
	}

	void OpenGL4_Renderer::Compute_Dispatch(const GFX_API::ComputeShader_Instance* CS, vec3 Dispatch_Groups) {
		Bind_ComputeInstance(CS);
		glDispatchCompute(Dispatch_Groups.x, Dispatch_Groups.y, Dispatch_Groups.z);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	void OpenGL4_Renderer::Bind_ComputeInstance(const GFX_API::ComputeShader_Instance* CS) {
		unsigned int PROGRAM_GLID = ((OGL4_ComputeShader*)GFXContentManager->Find_GFXComputeShader_byID(CS->ComputeShader)->GL_ID)->ShaderProgram_ID;
		glUseProgram(PROGRAM_GLID);
		//Bind uniforms!
		for (GFX_API::Material_Uniform UNIFORM : CS->UNIFORM_LIST) {
			Bind_Uniform(PROGRAM_GLID, &UNIFORM);
		}
		//Bind uniform textures!
		for (GFX_API::Texture_Access TEXTURE : CS->TEXTURE_LIST) {
			if (TEXTURE.ACCESS_TYPE == GFX_API::TEXTURE_ACCESS::SAMPLER_OPERATION && TEXTURE.TEXTURE_ID) {
				glActiveTexture(GL_TEXTURE0 + TEXTURE.BINDING_POINT);
				GFX_API::GFX_Texture* GFXTEXTURE = GFXContentManager->Find_GFXTexture_byID(TEXTURE.TEXTURE_ID);
				if (GFXTEXTURE) {
					glBindTexture(GL_TEXTURE_2D, *(unsigned int*)GFXTEXTURE->GL_ID);
				}
			}
		}
		//Bind image textures!
		for (GFX_API::Texture_Access TEXTURE : CS->TEXTURE_LIST) {
			if (TEXTURE.ACCESS_TYPE == GFX_API::TEXTURE_ACCESS::IMAGE_OPERATION && TEXTURE.TEXTURE_ID) {
				unsigned int TEXTURE_GLID = *(unsigned int*)GFXContentManager->Find_GFXTexture_byID(TEXTURE.TEXTURE_ID)->GL_ID;
				glBindImageTexture(TEXTURE.BINDING_POINT, TEXTURE_GLID, 0, GL_FALSE, 0, Find_OGLOperationType(TEXTURE.OP_TYPE), Find_ImageTexture_InternalFormat(TEXTURE.CHANNELs));
			}
		}
	}



	void Bind_Uniform(const unsigned int& PROGRAM_ID, const GFX_API::Material_Uniform* uniform) {
		//Get Uniform Location!
		int location = glGetUniformLocation(PROGRAM_ID, uniform->VARIABLE_NAME.c_str());
		if (location == -1) {
			//LOG_WARNING("Intended uniform: " + uniform->VARIABLE_NAME + " can't be found by Bind_Uniform()!");
			return;
		}
		if (uniform->DATA == nullptr) {
			//LOG_WARNING("Uniform: " + uniform->VARIABLE_NAME + " data isn't set!");
			return;
		}

		void* data = uniform->DATA;
		switch (uniform->VARIABLE_TYPE) {
		case GFX_API::DATA_TYPE::VAR_UINT32:
			glUniform1ui(location, *(unsigned int*)data);
			break;
		case GFX_API::DATA_TYPE::VAR_INT32:
			glUniform1i(location, *(int*)data);
			break;
		case GFX_API::DATA_TYPE::VAR_FLOAT32:
			glUniform1f(location, *(float*)data);
			break;
		case GFX_API::DATA_TYPE::VAR_VEC2:
			glUniform2f(location, ((vec2*)data)->x, ((vec2*)data)->y);
			break;
		case GFX_API::DATA_TYPE::VAR_VEC3:
			glUniform3f(location, ((vec3*)data)->x, ((vec3*)data)->y, ((vec3*)uniform->DATA)->z);
			break;
		case GFX_API::DATA_TYPE::VAR_VEC4:
			glUniform4f(location, ((vec4*)data)->x, ((vec4*)data)->y, ((vec4*)uniform->DATA)->z, ((vec4*)uniform->DATA)->w);
			break;
		case GFX_API::DATA_TYPE::VAR_MAT4x4:
			glUniformMatrix4fv(location, 1, GL_FALSE, (float*)data);
			break;
		default:
			LOG_ERROR("Error: Sending an unsupported uniform type! Nothing happened!");
			return;
		}
	}


	/*
	void OGL3_Renderer::Show_RenderTarget_onWindow(WINDOW* WINDOW_to_Display, RenderTarget* RenderTarget_to_Show) {
		//Bind Main Window Context and set other states!
		GFX_API::GFX_API_OBJ->Bind_Window_Context(WINDOW_to_Display);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//Bind Material Instance to display Render Target and bind Render Texture!
		Material_Instance* PP_TextureDisplay_MatInst = TAPIFILESYSTEM::Material_Instance::Find_MaterialInstance_byName("PP_TextureDisplay_inst");
		PP_TextureDisplay_MatInst->Set_Uniform_Data("Display_Texture", &RenderTarget_to_Show->ID);
		//PP_TextureDisplay_MatInst->Set_Uniform_Data("Display_Texture", &TAPIFILESYSTEM::Texture_Resource::ALL_TEXTUREs[0]->GL_ID);
		Bind_Material_Instance(PP_TextureDisplay_MatInst);

		//Draw Full-Screen Quad!
		//Note: Quad Mesh's VAO is 1, so we will directly access it! If you want to check it, you can 
		glBindVertexArray(1);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}


	//RENDERER FUNCTIONs

	void OGL3_Renderer::Bind_Framebuffer(Framebuffer* FRAMEBUFFER) {
		if (FRAMEBUFFER) {
			glBindFramebuffer(GL_FRAMEBUFFER, FRAMEBUFFER->ID);
		}
	}

	void OGL3_Renderer::Change_Framebuffer_Resolution(Framebuffer* FRAMEBUFFER, unsigned int width, unsigned int height) {
		Bind_Framebuffer(FRAMEBUFFER);
		for (Render_Target* rt : FRAMEBUFFER->BOUND_RTs) {
			glBindTexture(GL_TEXTURE_2D, rt->ID);
			glTexImage2D(GL_TEXTURE_2D, 0, Find_Texture_Format(rt->FORMAT), width, height, 0, Find_RenderTarget_Channel_Type(rt->FORMAT), Find_Texture_Value_Type(rt->FORMAT_VALUETYPE), NULL);
		}
		if (glGetError() != 0) {
			cout << "Error: When changing Framebuffer resolution!\n";
		}
		FRAMEBUFFER->WIDTH = width;
		FRAMEBUFFER->HEIGHT = height;
	}

	//Each Render Target Texture is in linear filter mode!
	void OGL3_Renderer::Create_RenderTarget(Framebuffer* FRAMEBUFFER, unsigned int width, unsigned int height, TuranAPI_ENUMs dimension, GFX_ENUM format, GFX_ENUM attachment, TuranAPI_ENUMs value_type) {
		Render_Target* gfx_rt = new GFX_Render_Target;
		//Note: Wrapping isn't supported for Render Target Textures
		gfx_rt->ATTACHMENT = attachment;
		gfx_rt->DIMENSION = dimension;
		gfx_rt->FORMAT = format;
		gfx_rt->FORMAT_VALUETYPE = value_type;

		int ATTACHMENT_TYPE = Find_Texture_Attachment_Type(attachment);
		int CHANNEL_TYPE = Find_RenderTarget_Channel_Type(format);
		int FORMAT = Find_Texture_Format(format);
		int DIMENSION = Find_Texture_Dimension(dimension);
		int VALUE_TYPE = Find_Texture_Value_Type(value_type);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		unsigned int rt_id;
		glGenTextures(1, &rt_id);
		glBindTexture(GL_TEXTURE_2D, rt_id);
		glTexImage2D(GL_TEXTURE_2D, 0, FORMAT, width, height, 0, CHANNEL_TYPE, VALUE_TYPE, NULL);
		cout << "Created RT ID:" << rt_id << endl;

		GFX_API::GFX_API_OBJ->Check_GL_Errors("When creating Render Target for Attachment: " + to_string(int(attachment)) + "!");

		gfx_rt->ID = rt_id;
		FRAMEBUFFER->BOUND_RTs.push_back(gfx_rt);
	}
	//Argument "name_of_framebuffer" is only used to cout meaningful info, nothing functional!
	void OGL3_Renderer::Check_ActiveFramebuffer_Status(string name_of_framebuffer) {
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status == GL_FRAMEBUFFER_COMPLETE) {
			cout << "Draw Pass: " << name_of_framebuffer << "'s framebuffer is successfully created!\n";
			return;
		}
		if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
			cout << "Error: Draw Pass: " << name_of_framebuffer << "'s framebuffer has incomplete attachment!\n";
		}
		if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
			cout << "Error: Draw Pass: " << name_of_framebuffer << "'s framebuffer has incomplete missing attachment!\n";
		}
		if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
			cout << "Error: Draw Pass: " << name_of_framebuffer << "'s framebuffer has unsupported type of attachment!\n";
		}
	}

	void OGL3_Renderer::Set_Depth_Test(GFX_ENUM DEPTH_TEST_MODE, GFX_ENUM DEPTH_BUFFER_MODE) {
	}

	GFX_Framebuffer* OGL3_Renderer::Create_Framebuffer(unsigned int width, unsigned int height) {
		GFX_Framebuffer* FRAMEBUFFER = new GFX_Framebuffer(width, height);
		glGenFramebuffers(1, &FRAMEBUFFER->ID);
		GFX_API::GFX_API_OBJ->Check_GL_Errors("After creating a Framebuffer!");
		return FRAMEBUFFER;
	}

	void OGL3_Renderer::Clear_RenderTarget(GFX_Framebuffer* FRAMEBUFFER, GFX_ENUM ATTACHMENT, vec3 CLEAR_COLOR) {
		if (Active_Framebuffer != FRAMEBUFFER) { Bind_Framebuffer(FRAMEBUFFER); }
		switch (ATTACHMENT) {
		case GFX_ENUM::GFX_TEXTURE_COLOR0_ATTACHMENT:
			glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			return;
		case GFX_ENUM::GFX_TEXTURE_DEPTH_ATTACHMENT:
			glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, 1);
			glClear(GL_DEPTH_BUFFER_BIT);
			return;
		default:
			cout << "Error: Clearing this Render Target attachment isn't supported for now!\n";
			cout << "Note: Don't forget, all of the COLOR BUFFER ATTACHMENTs are cleared when COLOR0_ATTACHMENT is cleared!\n";
			return;
		}
	}

	void OGL3_Renderer::Compile_MaterialType(string* compilation_status, const string* vertex_source, const string* fragment_source, unsigned int* program_id, unsigned int* vertex_id, unsigned int* fragment_id) {
		unsigned int vertexShader_o, fragmentShader_o, shaderProgram;
		//If we need to set argument ids in the end, we should set this true!
		bool set_ids = true;
		if (program_id == nullptr) {
			program_id = new unsigned int;
			vertex_id = new unsigned int;
			fragment_id = new unsigned int;
			vertexShader_o = glCreateShader(GL_VERTEX_SHADER);
			fragmentShader_o = glCreateShader(GL_FRAGMENT_SHADER);
			shaderProgram = glCreateProgram();
		}
		//If shader isn't compiled before, create and compile a new one!
		else if (*program_id == 0 && *vertex_id == 0 && *fragment_id == 0) {
			vertexShader_o = glCreateShader(GL_VERTEX_SHADER);
			fragmentShader_o = glCreateShader(GL_FRAGMENT_SHADER);
			shaderProgram = glCreateProgram();
			cout << "Created new shaders!\n";
		}
		//If shader is compiled before, bind them!
		else {
			vertexShader_o = *vertex_id;
			fragmentShader_o = *fragment_id;
			shaderProgram = *program_id;
			set_ids = false;
			return;
		}
		//Compile vertex shader and set ID!
		const char* vertex_source_cstr = vertex_source->c_str();
		glShaderSource(vertexShader_o, 1, &vertex_source_cstr, NULL);
		glCompileShader(vertexShader_o);

		//Check compile issues!
		int success;
		char vert_infolog[512];
		glGetShaderiv(vertexShader_o, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertexShader_o, 512, NULL, vert_infolog);
			*compilation_status = "Error: Vertex Shader couldn't compile!\n" + (string)"\n Error Info: " + vert_infolog + "\n" + "Vertex Source:\n" + *vertex_source;
			TuranAPI::Breakpoint();
			return;
		}


		//Compile fragment shader and set ID!
		const char* fragment_source_cstr = fragment_source->c_str();
		glShaderSource(fragmentShader_o, 1, &fragment_source_cstr, NULL);
		glCompileShader(fragmentShader_o);

		//Check compile issues!
		int frag_success;
		char frag_infolog[512];
		glGetShaderiv(fragmentShader_o, GL_COMPILE_STATUS, &frag_success);
		if (!frag_success) {
			glGetShaderInfoLog(fragmentShader_o, 512, NULL, frag_infolog);
			*compilation_status = "Error: Fragment Shader couldn't compile!\n" + (string)"\n Error Info: " + frag_infolog + "\n" + "Fragment Source:\n" + *fragment_source;
			TuranAPI::Breakpoint();
			return;
		}


		//Link Vertex and Fragment Shader to Shader Program and set ID
		glAttachShader(shaderProgram, vertexShader_o);
		glAttachShader(shaderProgram, fragmentShader_o);
		glLinkProgram(shaderProgram);

		//Check linking issues
		int link_success;
		char link_infolog[512];
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_success);
		if (!link_success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, link_infolog);
			*compilation_status = "Error: Shader Program couldn't link!\n" + (string)link_infolog + "\n";
			TuranAPI::Breakpoint();
			return;
		}
		if (set_ids) {
			*program_id = shaderProgram;
			*vertex_id = vertexShader_o;
			*fragment_id = fragmentShader_o;
		}
	}

	void OGL3_Renderer::Send_Textures_to_GPU(vector<TAPIFILESYSTEM::Texture_Resource*>* Textures) {
		unsigned int i = 0;
		for (TAPIFILESYSTEM::Texture_Resource* texture : *Textures) {
			if (texture->GL_ID != 0) {
				continue;
			}

			int texture_D = Find_Texture_Dimension(texture->Properties.DIMENSION);
			int Channel_Type = Find_Texture_Channel_Type(texture->Properties.CHANNEL_TYPE);
			int Value_Type = Find_Texture_Value_Type(texture->Properties.VALUE_TYPE);
			int Wrapping = Find_Texture_Wrapping(texture->Properties.WRAPPING);
			int Mipmap_Filtering = Find_Texture_Mipmap_Filtering(texture->Properties.MIPMAP_FILTERING);
			//All texture should be upscaled linearly!
			int Upscale_filtering = GL_LINEAR;


			glGenTextures(1, &texture->GL_ID);
			GFX_API::GFX_API_OBJ->Check_GL_Errors("After generating the texture!");
			glBindTexture(texture_D, texture->GL_ID);
			GFX_API::GFX_API_OBJ->Check_GL_Errors("After binding the texture!");


			// Set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(texture_D, GL_TEXTURE_WRAP_S, Wrapping);
			glTexParameteri(texture_D, GL_TEXTURE_WRAP_T, Wrapping);
			glTexParameteri(texture_D, GL_TEXTURE_MIN_FILTER, Mipmap_Filtering);
			glTexParameteri(texture_D, GL_TEXTURE_MAG_FILTER, Upscale_filtering);
			GFX_API::GFX_API_OBJ->Check_GL_Errors("After setting Wrapping and Filtering!");
			cout << "Set wrapping!\n";
			glTexImage2D(texture_D, 0, Channel_Type, texture->WIDTH, texture->HEIGHT, 0, Channel_Type, Value_Type, texture->DATA);
			glGenerateMipmap(texture_D);
			//glGenerateMipmap(texture_D);
			GFX_API::GFX_API_OBJ->Check_GL_Errors("After sending model texture: " + to_string(i) + "!");
			LOG_STATUS("TexImage2D is finished!");
			LOG_STATUS("Texture's GL_ID is: " + to_string(texture->GL_ID));
			i++;
		}
		LOG_STATUS("Send Texture Number: " + to_string(i));
	}

	void OGL3_Renderer::Send_StaticModelDatas_to_GPU(vector<TAPIFILESYSTEM::Static_Model_Data*>* StaticModel_Datas) {
		unsigned int sent_mesh_number = 0;
		//For each model in scene!
		for (Static_Model_Data* model_data : *StaticModel_Datas) {
			//Send each mesh part of the model to GPU!
			//Note: Each mesh has a different material according to .obj
			for (unsigned int mesh_index_in_model = 0; mesh_index_in_model < model_data->Get_Mesh_Number(); mesh_index_in_model++) {
				Static_Mesh_Data* mesh_data = model_data->Get_Mesh_byIndex(mesh_index_in_model);

				//If mesh is sent to GPU before, skip!
				if (mesh_data->GFXI_MESH != nullptr) {
					continue;
				}

				//Calculate the buffer size for the mesh!
				unsigned int Indices_Size = mesh_data->Get_Indice_Number() * sizeof(unsigned int);

				unsigned int Positions_Size = mesh_data->Get_Vertex_Number() * sizeof(vec3);
				unsigned int TextCoords_Size = mesh_data->Get_Vertex_Number() * sizeof(vec2);
				//Note: These have the same size with Positions_Size but I did this to understand the code better and debug easier!
				unsigned int Normals_Size = mesh_data->Get_Vertex_Number() * sizeof(vec3);
				unsigned int Tangents_Size = mesh_data->Get_Vertex_Number() * sizeof(vec3);
				unsigned int Bitangents_Size = mesh_data->Get_Vertex_Number() * sizeof(vec3);
				unsigned int Total_Vertex_Data_Size = Positions_Size + TextCoords_Size + Normals_Size + Tangents_Size + Bitangents_Size;

				GFXI_MESH* gfx_mesh = new GFXI_MESH;
				glGenVertexArrays(1, &gfx_mesh->VAO);
				glGenBuffers(1, &gfx_mesh->VBO);
				glGenBuffers(1, &gfx_mesh->EBO);
				gfx_mesh->Indices_Number = mesh_data->Get_Indice_Number();
				gfx_mesh->Vertex_Number = mesh_data->Get_Vertex_Number();

				glBindVertexArray(gfx_mesh->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, gfx_mesh->VBO);
				glBufferData(GL_ARRAY_BUFFER, Total_Vertex_Data_Size, NULL, GL_STATIC_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, NULL, Positions_Size, mesh_data->Get_Positions());
				glBufferSubData(GL_ARRAY_BUFFER, Positions_Size, TextCoords_Size, mesh_data->Get_TextCoords());
				glBufferSubData(GL_ARRAY_BUFFER, Positions_Size + TextCoords_Size, Normals_Size, mesh_data->Get_Normals());
				glBufferSubData(GL_ARRAY_BUFFER, Positions_Size + TextCoords_Size + Normals_Size, Tangents_Size, mesh_data->Get_Tangents());
				glBufferSubData(GL_ARRAY_BUFFER, Positions_Size + TextCoords_Size + Normals_Size + Tangents_Size, Bitangents_Size, mesh_data->Get_Bitangents());


				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gfx_mesh->EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices_Size, mesh_data->Get_Indices(), GL_STATIC_DRAW);

				//Position Attribute
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
				//TextCoord Attribute
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)Positions_Size);
				//Vertex Normal Attribute
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(Positions_Size + TextCoords_Size));
				//Tangent Attribute
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(Positions_Size + TextCoords_Size + Normals_Size));
				//Bitangent Attribute
				glEnableVertexAttribArray(4);
				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(Positions_Size + TextCoords_Size + Normals_Size + Tangents_Size));


				mesh_data->Set_GFXI_Mesh(gfx_mesh);
				mesh_data->GFXI_MESH = gfx_mesh;
				sent_mesh_number++;
			}
		}
	}


	void OGL3_Renderer::Bind_Material_Instance(TAPIFILESYSTEM::Material_Instance* MATERIAL_INST) {
		if (MATERIAL_INST->Material_Type->GFX_API != TuranAPI_ENUMs::OPENGL3) {
			cout << "Error: Bound Material Instance's Type isn't written in OpenGL3!\n";
			SLEEP_THREAD(10);
			return;
		}

		GFX_API::GFX_API_OBJ->Check_GL_Errors("Before binding shader!");
		unsigned int PROGRAM_ID = MATERIAL_INST->Material_Type->PROGRAM_ID;
		if (Active_ShaderProgram != MATERIAL_INST->Material_Type) { glUseProgram(PROGRAM_ID); }		//Bind Material Type (Shader Program), if it's not active!

		//Bind each uniform!
		for (Material_Uniform UNIFORM : MATERIAL_INST->UNIFORM_LIST) {
			Bind_Uniform(PROGRAM_ID, &UNIFORM);
		}
	}

	void OGL3_Renderer::Draw_Indexed_Objects(vector<void*> Vertex_Attributes, vector<unsigned int> Indice_Numbers) {
		for (unsigned int i = 0; i < Vertex_Attributes.size(); i++) {
			void* Attrib = Vertex_Attributes[i];
			unsigned int Indice_Number = Indice_Numbers[i];
			glBindVertexArray(*(unsigned int*)Attrib);
			glDrawElements(GL_TRIANGLES, Indice_Number, GL_UNSIGNED_INT, 0);
			GFX_API::GFX_API_OBJ->Check_GL_Errors("After drawing a mesh in G-Buffer Draw Pass!");
		}
	}













	*/
}