// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>

//GlM includes
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/ext/matrix_transform.hpp"

//Vector
#include <vector>

//For Image textures
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

//Camera
#include "Camera.cpp"


Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 45.0f);
float lastFrame = 0.0f;

std::string models_dir = "C:/Users/rgowt/source/repos/SubmarineChase/Models/";
std::string vertex_shader_dir = "C:/Users/rgowt/source/repos/SubmarineChase/Shaders/VertexShaders/";
std::string fragment_shader_dir = "C:/Users/rgowt/source/repos/SubmarineChase/Shaders/FragmentShaders/";
std::string texture_dir = "C:/Users/rgowt/source/repos/SubmarineChase/Textures/";

#pragma region SimpleTypes
struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texCoords;
};
struct ModelData
{
	size_t mPointCount = 0;
	std::vector<glm::vec3> mVertices;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec2> mTextureCoords;
};
struct Point {
	float x, y, z, rotation, scale_factor;
};
struct VertexBufferObject {
	GLuint vt, vn, vp;
};
#pragma endregion SimpleTypes

using namespace std;
GLuint sharkShaderProgramID, submarineShaderProgramID, terrainShaderProgramID, fanShaderProgramID, coralShaderProgramID, coral2ShaderProgramID, bubbleShaderProgramID, fishShaderProgramID, cylinderShaderProgramID, shellShaderProgramID, turtleShaderProgramID;
GLuint coral_texture_id, submarine_texture_id, fish_texture_id, coral2_texture_id, shell_texture_id, turtle_texture_id;

int width = 1600;
int height = 900;

GLuint loc1, loc2, loc3;
VertexBufferObject shark_vbo, submarine_vbo, fan_vbo, terrain_vbo, cylinder_vbo, coral_vbo, coral2_vbo, fish_vbo, bubble_vbo, shell_vbo, turtle_vbo;
ModelData shark_data, submarine_data, terrain_data, fan_data, coral_data, bubble_data, fish_data, cylinder_data, coral2_data, shell_data, turtle_data;

std::vector<Point> coral_points;
std::vector<Point> bubble_points;
std::vector<Point> fish_points;

float fan_rotation = 0;
float shark_y_angle = 0, shark_x_translate = 0, shark_y_translate = 0;
float submarine_x_translate = 0, submarine_y_translate = 0 ;

float bubble_translate = 0.0f, bubble_2_appearance_timer = 0.0f;
int scatter = 0;

float radius_shark = 42.0f * 3.0f;
float radius_submarine = 7.0f * 3.0f;
float angle_shark = 0.0f;
float angle_submarine = 3.14159 / 6;
float speed_submarine = 0.006f;
float speed_shark = 0.0062f;
float fish_displacement = 0.01f, fish_speed = 0.01f;
float turtle_displacement = 0.05f;

glm::vec3 flashlightPosition = glm::vec3(0.0f, 3.0f, 0.0f);
glm::vec3 flashlighRotation = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 fish_center = glm::vec3(-46.0f, -11.0, -46.0f);

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);
	

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(glm::vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(glm::vec3(vn->x, vn->y, vn->z));
			}			
			if (mesh->HasTextureCoords(0)) {
				glm::vec2 text_coords = glm::vec2(mesh->mTextureCoords[0][v_i].x, mesh->mTextureCoords[0][v_i].y);
				modelData.mTextureCoords.push_back(text_coords);
			}
		}

	}
	

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

std::string get_model_full_path(const char* filename) {
	std::string model_name_str = models_dir + (string)(filename);
	return model_name_str;
}

std::string get_vertex_shader_full_path(const char* filename) {
	std::string vertex_shader_name_str = vertex_shader_dir + (string)(filename);
	return vertex_shader_name_str;
}

std::string get_fragment_shader_full_path(const char* filename) {
	std::string fragment_shader_name_str = fragment_shader_dir + (string)(filename);
	return fragment_shader_name_str;
}

std::string get_texture_full_path(const char* filename) {
	std::string texture_name_str = texture_dir + (string)(filename);
	return texture_name_str;
}


#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char* vertex_shader, const char * fragment_shader)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, vertex_shader, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, fragment_shader, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	//glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

#pragma region VBO_FUNCTIONS
ModelData generateObjectBufferMesh(VertexBufferObject* vbo, const char* mesh_name) {
	
	ModelData mesh_data = load_mesh(mesh_name);	

	glGenBuffers(1, &vbo->vp);
	glBindBuffer(GL_ARRAY_BUFFER, vbo->vp);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(glm::vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &vbo->vn);
	glBindBuffer(GL_ARRAY_BUFFER, vbo->vn);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(glm::vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);


	glGenBuffers(1, &vbo->vt);
	glBindBuffer(GL_ARRAY_BUFFER, vbo->vt);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(glm::vec2), &mesh_data.mTextureCoords[0], GL_STATIC_DRAW);


	
	
	return mesh_data;
}
#pragma endregion VBO_FUNCTIONS


GLuint* TextureFromFile(GLuint textureID, const char* filename) {
	

	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(get_texture_full_path(filename).c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cerr << "Texture failed to load " << filename << std::endl;
		stbi_image_free(data);
	}

	return &textureID;
}

double distance(int x1, int x2, int y1, int y2) {
	return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

bool isValid(const Point& p, const std::vector<Point>& points, float min_distance) {
	for (const auto& point : points) {
		if (distance(p.x, point.x, p.y, point.y) < min_distance) {
			return false;
		}
	}
	return true;
}

void set_view_projection(GLuint shaderProgramId, glm::mat4 view, glm::mat4 persp_proj) {

	int view_mat_location = glGetUniformLocation(shaderProgramId, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramId, "proj");
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, glm::value_ptr(persp_proj));
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
}

void set_lighting(GLuint shaderProgramId) {
	glm::vec3 lightDirection = glm::vec3(-0.1f, -1.0f, -0.1f);
	glm::vec3 Ld = glm::vec3(1.0f, 1.0f, 1.0f); // Light source intensity

	int light_direction_location = glGetUniformLocation(shaderProgramId, "lightDirection");
	int ld_location = glGetUniformLocation(shaderProgramId, "Ld");
	glUniform3fv(light_direction_location, 1,  glm::value_ptr(lightDirection));
	glUniform3fv(ld_location, 1,  glm::value_ptr(Ld));

	
	
	float cutoff_angle = 30.0f * 3.14159f / 180.0;
	float cutoff_factor = cos(cutoff_angle);
	int flashlight_position = glGetUniformLocation(shaderProgramId, "flashlight_position");
	int flashlight_rotation = glGetUniformLocation(shaderProgramId, "flashlight_rotation");
	int cutoff_factor_location = glGetUniformLocation(shaderProgramId, "cutoff_factor");
	glUniform3fv(flashlight_position, 1, glm::value_ptr(flashlightPosition));
	glUniform3fv(flashlight_rotation, 1, glm::value_ptr(flashlighRotation));
	glUniform1f(cutoff_factor_location, cutoff_factor);

}

void bind_vao_and_vbo(GLuint ShaderProgramID, VertexBufferObject* vbo) {
	loc1 = glGetAttribLocation(ShaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(ShaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(ShaderProgramID, "vertex_texture");

	unsigned int vao = 0;
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo->vp);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo->vn);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vbo->vt);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

}
void display_bubbles(glm::mat4 view, glm::mat4 persp_proj) {

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(bubbleShaderProgramID);
	set_view_projection(bubbleShaderProgramID, view, persp_proj);
	set_lighting(bubbleShaderProgramID);
	glm::vec3 Kd = glm::vec3(1.0f);
	int matrix_location = glGetUniformLocation(bubbleShaderProgramID, "model");
	glUniform3fv(glGetUniformLocation(bubbleShaderProgramID, "viewPos"),1, value_ptr(camera.position));
	glUniform4fv(glGetUniformLocation(bubbleShaderProgramID, "bgColor"), 1, value_ptr(glm::vec4(0.0f, 0.09f, 0.211f, 0.2f)));

	bind_vao_and_vbo(bubbleShaderProgramID, &bubble_vbo);

	glUniform3fv(glGetUniformLocation(bubbleShaderProgramID, "Kd"), 1, value_ptr(Kd));

	for (int i = 0; i < bubble_points.size(); i++) {
		glm::mat4 model_bubble = glm::mat4(1.0f);

		model_bubble = scale(model_bubble, glm::vec3(bubble_points[i].scale_factor, bubble_points[i].scale_factor, bubble_points[i].scale_factor));
		model_bubble = translate(model_bubble, glm::vec3(bubble_points[i].x, bubble_points[i].y, bubble_points[i].z));



		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_bubble));
		glDrawArrays(GL_TRIANGLES, 0, bubble_data.mPointCount);
		bubble_points[i].y += 0.1f;
		if (bubble_points[i].y >= 20.0f) {
			bubble_points[i].y = -28.0f + static_cast<double>(std::rand()) / RAND_MAX * 20;
		}
	}

}

void display_cylinder(glm::mat4 view, glm::mat4 persp_proj) {
	glUseProgram(cylinderShaderProgramID);
	set_view_projection(cylinderShaderProgramID, view, persp_proj);
	set_lighting(cylinderShaderProgramID);
	glm::vec3 Kd = glm::vec3(0.29f, 0.176f, 0.118f);
	int matrix_location = glGetUniformLocation(cylinderShaderProgramID, "model");
	int rotation_angle_location = glGetUniformLocation(cylinderShaderProgramID, "rotation_angle");
	
	bind_vao_and_vbo(cylinderShaderProgramID, &cylinder_vbo);

	glm::mat4 model_cylinder = glm::mat4(1.0);
	model_cylinder = translate(model_cylinder, flashlightPosition);
	model_cylinder = rotate(model_cylinder, -cos(angle_submarine), glm::vec3(1.0f, 0.0f, 0.0f));
	model_cylinder = rotate(model_cylinder, -sin(angle_submarine), glm::vec3(0.0f, 0.0f, 1.0f));
	model_cylinder = rotate(model_cylinder, -1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_cylinder));
	glUniform3fv(glGetUniformLocation(cylinderShaderProgramID, "Kd"), 1, value_ptr(Kd));
	glDrawArrays(GL_TRIANGLES, 0, cylinder_data.mPointCount);

}
void display_shark(glm::mat4 view, glm::mat4 persp_proj) {

	glUseProgram(sharkShaderProgramID);
	set_view_projection(sharkShaderProgramID, view, persp_proj);
	set_lighting(sharkShaderProgramID);
	int matrix_location = glGetUniformLocation(sharkShaderProgramID, "model");
	int rotation_angle_location = glGetUniformLocation(sharkShaderProgramID, "rotation_angle");
	

	bind_vao_and_vbo(sharkShaderProgramID, &shark_vbo);

	glm::mat4 model_shark = glm::mat4(1.0);
	model_shark = translate(model_shark, glm::vec3(shark_x_translate / 2, -5.0f, shark_y_translate / 2));
	model_shark = scale(model_shark, glm::vec3(0.5f, 0.5f, 0.5f));
	
	model_shark = rotate(model_shark, glm::radians(shark_y_angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model_shark = rotate(model_shark, -angle_shark, glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_shark));
	glUniform1f(rotation_angle_location, shark_y_angle);
	glDrawArrays(GL_TRIANGLES, 0, shark_data.mPointCount);

	
	glm::mat4 model_shark_2 = glm::mat4(1.0);
	model_shark_2 = translate(model_shark_2, glm::vec3(0.0f, 5.0f, -4.0f));
	model_shark_2 = model_shark * model_shark_2;

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_shark_2));
	glDrawArrays(GL_TRIANGLES, 0, shark_data.mPointCount);

	

}
void display_fish(glm::mat4 view, glm::mat4 persp_proj) {
	glUseProgram(fishShaderProgramID);
	int matrix_location = glGetUniformLocation(fishShaderProgramID, "model");
	set_view_projection(fishShaderProgramID, view, persp_proj);
	set_lighting(fishShaderProgramID);
	glm::vec3 Kd = glm::vec3(1.0f);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fish_texture_id);
	glUniform1i(glGetUniformLocation(fishShaderProgramID, "texture"), 0);
	bind_vao_and_vbo(fishShaderProgramID, &fish_vbo);

	glUniform3fv(glGetUniformLocation(fishShaderProgramID, "Kd"), 1, value_ptr(Kd));
	//
	
	float fish_scale = 0.5f;
	for (int i = 0; i < fish_points.size(); i++) {
		glm::mat4 model_fish = glm::mat4(1.0);
		model_fish = scale(model_fish, glm::vec3(fish_scale, fish_scale, fish_scale));
		model_fish = rotate(model_fish, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model_fish = rotate(model_fish, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 fish_position = glm::vec3(fish_points[i].x, fish_points[i].y, fish_points[i].z) / fish_scale;
		model_fish = translate(model_fish, fish_position);
		
		model_fish = rotate(model_fish, glm::radians(fish_points[i].rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		
		if (scatter == 0 || scatter == 1) {
			model_fish = translate(model_fish, glm::vec3(0.0f, -fish_displacement, 0.0f));
		}		
		else if (scatter == 2) {
			model_fish = rotate(model_fish, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			model_fish = translate(model_fish, glm::vec3(0.0f, fish_displacement, 0.0f));
			
		}
			
		
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_fish));
		glDrawArrays(GL_TRIANGLES, 0, fish_data.mPointCount);
	}
	

}

void display_submarine(glm::mat4 view, glm::mat4 persp_proj) {

	glUseProgram(submarineShaderProgramID);
	int matrix_location = glGetUniformLocation(submarineShaderProgramID, "model");
	set_view_projection(submarineShaderProgramID, view, persp_proj);
	set_lighting(submarineShaderProgramID);
	glm::vec3 Kd = glm::vec3(1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, submarine_texture_id);
	glUniform1i(glGetUniformLocation(submarineShaderProgramID, "texture"), 0);

	bind_vao_and_vbo(submarineShaderProgramID, &submarine_vbo);

	glm::mat4 model_submarine = glm::mat4(1.0);
	model_submarine = translate(model_submarine, glm::vec3(submarine_x_translate * 3, -3.0f, submarine_y_translate * 3));
	model_submarine = scale(model_submarine, glm::vec3(3.0f, 3.0f, 3.0f));
	model_submarine = rotate(model_submarine, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	
	//model_submarine = translate(model_submarine, glm::vec3(submarine_x_translate,submarine_y_translate, 1.0f ));
	model_submarine = rotate(model_submarine, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model_submarine = rotate(model_submarine, angle_submarine, glm::vec3(0.0f, 0.0f, 1.0f));

	glUniform3fv(glGetUniformLocation(submarineShaderProgramID, "Kd"), 1, value_ptr(Kd));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_submarine));
	glDrawArrays(GL_TRIANGLES, 0, submarine_data.mPointCount);

	
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_submarine));
	glDrawArrays(GL_TRIANGLES, 0, submarine_data.mPointCount);


	glUseProgram(fanShaderProgramID);
	Kd = glm::vec3(0.29f, 0.176f, 0.118f);
	matrix_location = glGetUniformLocation(fanShaderProgramID, "model");
	set_view_projection(fanShaderProgramID, view, persp_proj);
	set_lighting(fanShaderProgramID);


	bind_vao_and_vbo(fanShaderProgramID, &fan_vbo);


	glm::mat4 model_fan = glm::mat4(1.0);
	model_fan = rotate(model_fan, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model_fan = translate(model_fan, glm::vec3(0.0f, 1.22f, 0.45f));
	model_fan = rotate(model_fan, glm::radians(fan_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	model_fan = model_submarine * model_fan; // Attach the fan to the submarine


	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_fan));
	glUniform3fv(glGetUniformLocation(fanShaderProgramID, "Kd"), 1, value_ptr(Kd));
	glDrawArrays(GL_TRIANGLES, 0, fan_data.mPointCount);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(bubbleShaderProgramID);
	set_view_projection(bubbleShaderProgramID, view, persp_proj);
	set_lighting(bubbleShaderProgramID);
	Kd = glm::vec3(1.0f);
	matrix_location = glGetUniformLocation(bubbleShaderProgramID, "model");
	glUniform3fv(glGetUniformLocation(bubbleShaderProgramID, "viewPos"), 1, value_ptr(camera.position));
	glUniform4fv(glGetUniformLocation(bubbleShaderProgramID, "bgColor"), 1, value_ptr(glm::vec4(0.035f, 0.21f, 0.5f, 0.3f)));

	bind_vao_and_vbo(bubbleShaderProgramID, &bubble_vbo);

	glm::mat4 model_bubble = glm::mat4(1.0f);
	if (bubble_translate == 0.0f) {
		model_bubble = glm::mat4(1.0f);
	}
	model_bubble = scale(model_bubble, glm::vec3(0.1f, 0.1f, 0.1f));
	model_bubble = translate(model_bubble, glm::vec3(16.0f, 0.0f, 5.0f));
	model_bubble = model_submarine * model_bubble;
	model_bubble = translate(model_bubble, glm::vec3(bubble_translate/2, 0.0f, -bubble_translate));
	
	if (bubble_translate >= 6.5f) {
		model_bubble = glm::mat4(0.0f);
		bubble_translate = 0.0f;
		bubble_2_appearance_timer = 0.0f;
	}
	


	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_bubble));
	glUniform3fv(glGetUniformLocation(bubbleShaderProgramID, "Kd"), 1, value_ptr(Kd));
	glDrawArrays(GL_TRIANGLES, 0, bubble_data.mPointCount);

	
	if (bubble_2_appearance_timer > 400) {
		glm::mat4 model_bubble_2 = glm::mat4(1.0f);
		model_bubble_2 = translate(model_bubble_2, glm::vec3(-2.0f, 0.0f, 2.0f));
		model_bubble_2 = model_bubble * model_bubble_2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_bubble_2));

		glDrawArrays(GL_TRIANGLES, 0, bubble_data.mPointCount);
	}
	if (bubble_2_appearance_timer > 800) {
		glm::mat4 model_bubble_3 = glm::mat4(1.0f);
		model_bubble_3 = translate(model_bubble_3, glm::vec3(-4.0f, 0.0f, 4.0f));
		model_bubble_3 = model_bubble * model_bubble_3;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_bubble_3));

		glDrawArrays(GL_TRIANGLES, 0, bubble_data.mPointCount);
	}
	
}

void display_terrain(glm::mat4 view, glm::mat4 persp_proj, glm::vec3 position) {
	glUseProgram(terrainShaderProgramID);
	glm::vec3 Kd = glm::vec3(0.571f, 0.285f, 0.015f);
	set_view_projection(terrainShaderProgramID, view, persp_proj);
	set_lighting(terrainShaderProgramID);
	int matrix_location = glGetUniformLocation(terrainShaderProgramID, "model");
	bind_vao_and_vbo(terrainShaderProgramID, &terrain_vbo);
		
	glm::mat4 model_terrain = glm::mat4(1.0);
	model_terrain = translate(model_terrain, position); 
	model_terrain = scale(model_terrain, glm::vec3(4.204f, 0.7f, 4.204f));

	
	glUniform3fv(glGetUniformLocation(terrainShaderProgramID, "Kd"), 1,value_ptr(Kd));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_terrain));
	glDrawArrays(GL_TRIANGLES, 0, terrain_data.mPointCount);

}

void display_corals(glm::mat4 view, glm::mat4 persp_proj) {
	glUseProgram(coralShaderProgramID);
	int matrix_location = glGetUniformLocation(coralShaderProgramID, "model");
	set_view_projection(coralShaderProgramID, view, persp_proj);
	set_lighting(coralShaderProgramID);
	glm::vec3 Kd = glm::vec3(1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, coral_texture_id);
	glUniform1i(glGetUniformLocation(coralShaderProgramID, "texture"), 0);

	bind_vao_and_vbo(coralShaderProgramID, &coral_vbo);
	glUniform3fv(glGetUniformLocation(coralShaderProgramID, "Kd"), 1, value_ptr(Kd));

	for (int i = 1; i < coral_points.size() / 2; i++) {
		glm::mat4 model_coral = glm::mat4(1.0);

		model_coral = translate(model_coral, glm::vec3(coral_points[i].x, -12.2 , coral_points[i].y));
		model_coral = scale(model_coral, glm::vec3(coral_points[i].scale_factor, coral_points[i].scale_factor, coral_points[i].scale_factor));
		
		model_coral = rotate(model_coral, glm::radians(coral_points[i].rotation), glm::vec3(0.0f, 1.0f, 0.0f));



		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_coral));
		glDrawArrays(GL_TRIANGLES, 0, coral_data.mPointCount);
	}

	glUseProgram(coral2ShaderProgramID);
	matrix_location = glGetUniformLocation(coral2ShaderProgramID, "model");
	set_view_projection(coral2ShaderProgramID, view, persp_proj);
	set_lighting(coral2ShaderProgramID);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, coral2_texture_id);
	glUniform1i(glGetUniformLocation(coral2ShaderProgramID, "texture"), 0);

	bind_vao_and_vbo(coral2ShaderProgramID, &coral2_vbo);
	glUniform3fv(glGetUniformLocation(coral2ShaderProgramID, "Kd"), 1, value_ptr(Kd));

	for (int i = coral_points.size() / 2; i < coral_points.size(); i++) {
		glm::mat4 model_coral = glm::mat4(1.0);
		model_coral = translate(model_coral, glm::vec3(coral_points[i].x, -12.2 , coral_points[i].y));
		model_coral = scale(model_coral, glm::vec3(coral_points[i].scale_factor * 5, coral_points[i].scale_factor * 5, coral_points[i].scale_factor * 5));
		
		model_coral = rotate(model_coral, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model_coral = rotate(model_coral, glm::radians(coral_points[i].rotation), glm::vec3(0.0f, 0.0f, 1.0f));



		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_coral));
		glDrawArrays(GL_TRIANGLES, 0, coral2_data.mPointCount);
	}
}

void display_shell(glm::mat4 view, glm::mat4 persp_proj) {
	glUseProgram(shellShaderProgramID);
	int matrix_location = glGetUniformLocation(shellShaderProgramID, "model");
	set_view_projection(shellShaderProgramID, view, persp_proj);
	set_lighting(shellShaderProgramID);
	glm::vec3 Kd = glm::vec3(1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shell_texture_id);
	glUniform1i(glGetUniformLocation(shellShaderProgramID, "texture"), 0);

	bind_vao_and_vbo(shellShaderProgramID, &shell_vbo);
	glUniform3fv(glGetUniformLocation(shellShaderProgramID, "Kd"), 1, value_ptr(Kd));

	glm::mat4 model_shell = glm::mat4(1.0);
	model_shell = translate(model_shell, fish_center);
	model_shell = translate(model_shell, glm::vec3(0.0f, -1.0f, 0.0f));
	model_shell = scale(model_shell, glm::vec3(0.5f, 0.5f, 0.5f));

	model_shell = rotate(model_shell, 3.14159f, glm::vec3(0.0f, 1.0f, 0.0f));



	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_shell));
	glDrawArrays(GL_TRIANGLES, 0, shell_data.mPointCount);
}

void display_turtle(glm::mat4 view, glm::mat4 persp_proj) {
	glUseProgram(turtleShaderProgramID);
	int matrix_location = glGetUniformLocation(turtleShaderProgramID, "model");
	set_view_projection(turtleShaderProgramID, view, persp_proj);
	set_lighting(turtleShaderProgramID);
	glm::vec3 Kd = glm::vec3(1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, turtle_texture_id);
	glUniform1i(glGetUniformLocation(turtleShaderProgramID, "texture"), 0);

	bind_vao_and_vbo(turtleShaderProgramID, &turtle_vbo);
	glUniform3fv(glGetUniformLocation(turtleShaderProgramID, "Kd"), 1, value_ptr(Kd));

	glm::mat4 model_turtle = glm::mat4(1.0);
	//model_turtle = translate(model_turtle, fish_center);
	model_turtle = translate(model_turtle, glm::vec3(35.0f, -7.0f, -35.0f));
	model_turtle = scale(model_turtle, glm::vec3(5.0f, 5.0f, 5.0f));
		  
	model_turtle = rotate(model_turtle, -3.14159f / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	model_turtle = rotate(model_turtle, glm::radians(-120.0f), glm::vec3(0.00f, 0.0f, 1.0f));
	model_turtle = rotate(model_turtle, glm::radians(-2.0f), glm::vec3(0.00f, 1.0f, 0.0f));
	model_turtle = translate(model_turtle, glm::vec3(turtle_displacement, 0.0f, 0.0f));


	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_turtle));
	glDrawArrays(GL_TRIANGLES, 0, turtle_data.mPointCount);

	glm::mat4 model_turtle_2 = glm::mat4(1.0);
	model_turtle_2 = translate(model_turtle_2, glm::vec3(0.0f, 1.0f, 1.0f));
	model_turtle_2 = model_turtle * model_turtle_2;
	model_turtle_2 = rotate(model_turtle_2, glm::radians(2.0f), glm::vec3(0.00f, 1.0f, 0.0f));

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_turtle_2));
	glDrawArrays(GL_TRIANGLES, 0, turtle_data.mPointCount);

	glm::mat4 model_turtle_3 = glm::mat4(1.0);
	model_turtle_3 = translate(model_turtle_3, glm::vec3(-1.0f, -0.5f, 1.0f));
	model_turtle_3 = model_turtle * model_turtle_3;
	model_turtle_3 = rotate(model_turtle_3, glm::radians(-1.0f), glm::vec3(0.00f, 1.0f, 0.0f));

	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, value_ptr(model_turtle_3));
	glDrawArrays(GL_TRIANGLES, 0, turtle_data.mPointCount);
}

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.0f, 0.09f, 0.211f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	glm::mat4 view = camera.getViewMatrix();
	glm::mat4 persp_proj = camera.getProjectionMatrix(aspectRatio);

	display_shark(view, persp_proj);


	float chunkSize = 100.0f;
	float renderDistance = 300.0f; // how far to render the terrain
	glm::vec3 cameraPosition = camera.position; // Get the camera position

	// Calculate the number of chunks to render in each direction
	int startX = floor((cameraPosition.x - renderDistance) / chunkSize);
	int endX = floor((cameraPosition.x + renderDistance) / chunkSize);
	int startZ = floor((cameraPosition.z - renderDistance) / chunkSize);
	int endZ = floor((cameraPosition.z + renderDistance) / chunkSize);

	// Render terrain chunks
	for (int x = startX; x <= endX; ++x) {
		for (int z = startZ; z <= endZ; ++z) {
			glm::vec3 chunkPosition(x * chunkSize, -12.0f, z * chunkSize);
			display_terrain(view, persp_proj, chunkPosition);
		}
	}

	display_fish(view, persp_proj);
	display_shell(view, persp_proj);
	display_turtle(view, persp_proj);
	display_corals(view, persp_proj);
	display_cylinder(view, persp_proj);


	display_submarine(view, persp_proj);

	display_bubbles(view, persp_proj);

	flashlightPosition = glm::vec3(submarine_x_translate * 3.0f - 4.5f * sin(angle_submarine), -3.5f, submarine_y_translate * 3.0f + 4.5 * cos(angle_submarine));
	flashlighRotation = glm::vec3(-sin(angle_submarine), -1.0f, cos(angle_submarine));
	

	if (scatter == 0 && distance(flashlightPosition.x, fish_center.x, flashlightPosition.z, fish_center.z) < 35.0f) {
		scatter = 1;
		fish_speed = 0.2f;
	}
	if (scatter == 1 && distance(flashlightPosition.x, fish_center.x, flashlightPosition.z, fish_center.z) > 35.0f) {
		scatter = 2;
		fish_speed = -0.07f;
	}
	if (fish_displacement <= 0) {
		fish_displacement = 0;
		fish_speed = 0.01f;
		scatter = 0;
	}
	
	if (shark_x_translate != 0.0f && distance(submarine_x_translate * 3.0f, shark_x_translate / 2.0f, submarine_y_translate * 3.0f, shark_y_translate / 2.0f) < 5.5f) {
		speed_submarine *= 1.1;
	}
	//printf("%f, %f, %f\n", flashlightPosition.x, flashlightPosition.y, flashlightPosition.z);
	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time);
	last_time = curr_time;

	bubble_translate += delta / 200;
	bubble_2_appearance_timer += delta;
	shark_y_angle = 5.0 * sin(curr_time/200);
	
	shark_x_translate = radius_shark * cos(angle_shark);
	shark_y_translate = radius_shark * sin(angle_shark);

	fish_displacement += fish_speed;
	turtle_displacement += 0.01f;

	submarine_x_translate = radius_submarine * cos(angle_submarine);
	submarine_y_translate = radius_submarine * sin(angle_submarine);
	angle_shark += speed_shark;
	angle_submarine += speed_submarine;
	if (angle_shark > 2 * 3.14159f)  // Keep the angle in range [0, 2*Pi]
		angle_shark -= 2 * 3.14159f;
	
	fan_rotation += delta / 2;
	if (fan_rotation > 360) 
		fan_rotation -= 360;
	
	

	if (angle_submarine > 2 * 3.14159f)  // Keep the angle in range [0, 2*Pi]
		angle_submarine -= 2 * 3.14159f;	
	
	
	// Draw the next frame
	glutPostRedisplay();
}

void generateCoralPoints(int numCorals) {

	std::srand(std::time(nullptr));
	Point shell_point;
	shell_point.x = -46.0f;
	shell_point.y = -46.0f;
	shell_point.scale_factor = 0.1f;
	shell_point.rotation = 90.0f;
	coral_points.push_back(shell_point);
	//const int numCorals = 200;
	while (coral_points.size() < numCorals) {
		Point p;
		p.x = -100.0f + static_cast<double>(std::rand()) / RAND_MAX * 200;
		p.y = -100.0f + static_cast<double>(std::rand()) / RAND_MAX * 200;
		p.rotation = static_cast<double>(std::rand()) / RAND_MAX * 360;
		p.scale_factor = 0.02 + static_cast<double>(std::rand()) / RAND_MAX * 0.1;
		// Check if the point is valid
		if (isValid(p, coral_points, 10.0f)) {
			coral_points.push_back(p);
		}
	}
}

void generateBubblePoints(int numBubbles) {
	std::srand(std::time(nullptr));
	while (bubble_points.size() < numBubbles) {
		Point p;
		p.x = -150.0f + static_cast<double>(std::rand()) / RAND_MAX * 300;
		p.y = -150.0f + static_cast<double>(std::rand()) / RAND_MAX * 300;
		p.z = -28.0f +  static_cast<double>(std::rand()) / RAND_MAX * 20;
		p.scale_factor = 0.02 + static_cast<double>(std::rand()) / RAND_MAX * 0.07;
		// Check if the point is valid
		if (isValid(p, bubble_points, 10.0f)) {
			float temp = p.y;
			p.y = p.z;
			p.z = temp;
			bubble_points.push_back(p);
		}
	}
}

void generateFishPoints() {
	std::srand(std::time(nullptr));
	while (fish_points.size() < 11) {
		Point p;
		p.x = 44.0f + static_cast<double>(std::rand()) / RAND_MAX * 4;
		p.y = -44.0f - static_cast<double>(std::rand()) / RAND_MAX * 4;
		p.z = -9.0f - static_cast<double>(std::rand()) / RAND_MAX * 4;
		p.rotation = static_cast<double>(std::rand()) / RAND_MAX * 360;
		fish_points.push_back(p);
	}
}


void init()
{
	
	
	sharkShaderProgramID = CompileShaders(get_vertex_shader_full_path("sharkVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleFragmentShader.txt").c_str());
	shark_data = generateObjectBufferMesh(&shark_vbo, get_model_full_path("Shark.dae").c_str() );
	

	submarineShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleTextureFragmentShader.txt").c_str());
	submarine_data = generateObjectBufferMesh(&submarine_vbo,  get_model_full_path("submarine.dae").c_str());
	submarine_texture_id = *(TextureFromFile(2, "submarine_diffuse.jpeg"));
	
	fanShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleFragmentShader.txt").c_str());
	fan_data = generateObjectBufferMesh(&fan_vbo, get_model_full_path("fan.dae").c_str());
	
	
	terrainShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleFragmentShader.txt").c_str());
	terrain_data = generateObjectBufferMesh(&terrain_vbo, get_model_full_path("terrain.dae").c_str());
	
	
	coralShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleTextureFragmentShader.txt").c_str());
	coral_data = generateObjectBufferMesh(&coral_vbo, get_model_full_path("coral.dae").c_str());
	coral_texture_id = *(TextureFromFile(1, "coral_diffuse.jpg"));

	coral2ShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleTextureFragmentShader.txt").c_str());
	coral2_data = generateObjectBufferMesh(&coral2_vbo, get_model_full_path("coral2.dae").c_str());
	coral2_texture_id = *(TextureFromFile(1, "coral2_diffuse.jpg"));
	

	bubbleShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("bubbleFragmentShader.txt").c_str());;
	bubble_data = generateObjectBufferMesh(&bubble_vbo, get_model_full_path("bubble.dae").c_str());;

	fishShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleTextureFragmentShader.txt").c_str());;
	fish_data = generateObjectBufferMesh(&fish_vbo, get_model_full_path("fish.dae").c_str());;
	fish_texture_id = *(TextureFromFile(3, "fish_diffuse.jpg"));

	cylinderShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleFragmentShader.txt").c_str());
	cylinder_data = generateObjectBufferMesh(&cylinder_vbo, get_model_full_path("cylinder.dae").c_str());

	shellShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleTextureFragmentShader.txt").c_str());
	shell_data = generateObjectBufferMesh(&shell_vbo, get_model_full_path("shell.dae").c_str());
	shell_texture_id = *(TextureFromFile(1, "shell_diffuse.jpg"));

	turtleShaderProgramID = CompileShaders(get_vertex_shader_full_path("simpleVertexShader.txt").c_str(), get_fragment_shader_full_path("simpleTextureFragmentShader.txt").c_str());
	turtle_data = generateObjectBufferMesh(&turtle_vbo, get_model_full_path("turtle.dae").c_str());
	turtle_texture_id = *(TextureFromFile(1, "turtle_diffuse.jpg"));

	generateCoralPoints(100);
	generateBubblePoints(70);
	generateFishPoints();
	
	
}

bool mousePressed = false;
float lastX, lastY;

void processNormalKeys(unsigned char key, int x, int y) {
	float deltaTime = 0.1f;
	camera.processKeyboard(key, deltaTime);
	glutPostRedisplay();
}

void mouseButtonCallback(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			mousePressed = true;
			lastX = x;
			lastY = y;
		}
		else if (state == GLUT_UP) {
			mousePressed = false;
		}
	}
}


void mouseMotionCallback(int x, int y) {
	if (mousePressed) {
		float xOffset = x - lastX;
		float yOffset = lastY - y;

		lastX = x;
		lastY = y;

		// adapt the perpective by the movement
		camera.processMouseMovement(xOffset, yOffset);
		glutPostRedisplay();
	}
}


int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Shark Attack");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(processNormalKeys);
	glutMouseFunc(mouseButtonCallback);
	glutMotionFunc(mouseMotionCallback);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
