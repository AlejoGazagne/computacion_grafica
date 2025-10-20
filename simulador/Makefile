INC_DIRS = \
	./src \
	./src/core \
	./src/graphics \
	./src/graphics/shaders \
	./src/graphics/textures \
	./src/graphics/rendering \
	./src/graphics/skybox \
	./src/scene \
	./src/input \
	./src/ui \
	./src/utils \
	./include \
	/usr/include/glmLIB_DIRS = \
./include

SRC_DIR = \
./src

PROJECT_NAME = OGL-Engine
MAIN_CXX = main
GLAD_CXX = glad-460

# Objetos modulares adicionales del sistema
CORE_CXX = core/opengl_context
SHADER_MANAGER_CXX = graphics/shaders/shader_manager
TEXTURE_MANAGER_CXX = graphics/textures/texture_manager
BUFFER_OBJECTS_CXX = graphics/rendering/buffer_objects
SKYBOX_CXX = graphics/skybox/skybox
MESH_CXX = scene/mesh
CAMERA_CXX = scene/camera
TERRAIN_CXX = scene/terrain
MODEL_CXX = scene/model
INSTANCING_CXX = scene/instancing
INPUT_MANAGER_CXX = input/input_manager
HUD_INSTRUMENT_CXX = ui/hud_instrument
HUD_CXX = ui/hud
BANK_ANGLE_CXX = ui/bank_angle
PITCH_LADDER_CXX = ui/pitch_ladder
TEXT_RENDERER_CXX = ui/text_renderer
TEXT_RENDERER_ATLAS_CXX = ui/text_renderer_atlas
OBJ_LOADER_CXX = utils/obj_loader

# Lista de objetos adicionales
ADDITIONAL_OBJS = \
	$(BUILD_DIR)/$(CORE_CXX).o \
	$(BUILD_DIR)/$(SHADER_MANAGER_CXX).o \
	$(BUILD_DIR)/$(TEXTURE_MANAGER_CXX).o \
	$(BUILD_DIR)/$(BUFFER_OBJECTS_CXX).o \
	$(BUILD_DIR)/$(SKYBOX_CXX).o \
	$(BUILD_DIR)/$(MESH_CXX).o \
	$(BUILD_DIR)/$(CAMERA_CXX).o \
	$(BUILD_DIR)/$(TERRAIN_CXX).o \
	$(BUILD_DIR)/$(MODEL_CXX).o \
	$(BUILD_DIR)/$(INSTANCING_CXX).o \
	$(BUILD_DIR)/$(INPUT_MANAGER_CXX).o \
	$(BUILD_DIR)/$(HUD_INSTRUMENT_CXX).o \
	$(BUILD_DIR)/$(HUD_CXX).o \
	$(BUILD_DIR)/$(BANK_ANGLE_CXX).o \
	$(BUILD_DIR)/$(PITCH_LADDER_CXX).o \
	$(BUILD_DIR)/$(TEXT_RENDERER_CXX).o \
	$(BUILD_DIR)/$(TEXT_RENDERER_ATLAS_CXX).o \
	$(BUILD_DIR)/$(OBJ_LOADER_CXX).o# Compile with debug symbols
USERCPPFLAGS = -g -Wall -Wextra

include ./Makefile.master
