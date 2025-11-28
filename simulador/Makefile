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
	./src/hud \
	./src/hud/components \
	./src/hud/instruments \
	./src/mission \
	./src/utils \
	./src/physics \
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
CHUNKED_TERRAIN_CXX = scene/chunked_terrain
MODEL_CXX = scene/model
INPUT_MANAGER_CXX = input/input_manager
BANK_ANGLE_CXX = hud/instruments/bank_angle
PITCH_LADDER_CXX = hud/instruments/pitch_ladder
ALTIMETER_CXX = hud/instruments/altimeter
SPEED_INDICATOR_CXX = hud/instruments/speed_indicator
VERTICAL_SPEED_INDICATOR_CXX = hud/instruments/vertical_speed_indicator
WAYPOINT_INDICATOR_CXX = hud/instruments/waypoint_indicator
TEXT_RENDERER_CXX = hud/text_renderer
ASSIMP_LOADER_CXX = utils/model_loader
HUD_INSTRUMENTBASE_CXX = include/hud/instrumentbase

# Mission system
MISSION_RUNTIME_CXX = mission/mission_runtime
WAYPOINT_SYSTEM_CXX = mission/waypoint_system
WAYPOINT_RENDERER_CXX = mission/waypoint_renderer

# Physics system (FDM)
FLIGHT_DYNAMICS_CXX = physics/flight_dynamics
AERODYNAMICS_MODEL_CXX = dlfdm/aerodynamicsmodel
AIRCRAFT_DYNAMICS_CXX = dlfdm/aircraftdynamics
FDM_SOLVER_CXX = dlfdm/fdmsolver

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
	$(BUILD_DIR)/$(CHUNKED_TERRAIN_CXX).o \
	$(BUILD_DIR)/$(MODEL_CXX).o \
	$(BUILD_DIR)/$(INPUT_MANAGER_CXX).o \
	$(BUILD_DIR)/$(BANK_ANGLE_CXX).o \
	$(BUILD_DIR)/$(PITCH_LADDER_CXX).o \
	$(BUILD_DIR)/$(ALTIMETER_CXX).o \
	$(BUILD_DIR)/$(SPEED_INDICATOR_CXX).o \
	$(BUILD_DIR)/$(VERTICAL_SPEED_INDICATOR_CXX).o \
	$(BUILD_DIR)/$(WAYPOINT_INDICATOR_CXX).o \
	$(BUILD_DIR)/$(TEXT_RENDERER_CXX).o \
	$(BUILD_DIR)/$(ASSIMP_LOADER_CXX).o \
	$(BUILD_DIR)/$(HUD_INSTRUMENTBASE_CXX).o \
	$(BUILD_DIR)/$(MISSION_RUNTIME_CXX).o \
	$(BUILD_DIR)/$(WAYPOINT_SYSTEM_CXX).o \
	$(BUILD_DIR)/$(WAYPOINT_RENDERER_CXX).o \
	$(BUILD_DIR)/$(FLIGHT_DYNAMICS_CXX).o \
	$(BUILD_DIR)/$(AERODYNAMICS_MODEL_CXX).o \
	$(BUILD_DIR)/$(AIRCRAFT_DYNAMICS_CXX).o \
	$(BUILD_DIR)/$(FDM_SOLVER_CXX).o

# Compile with debug symbols
USERCPPFLAGS = -g -Wall -Wextra

include ./Makefile.master
