#include <vector>
#include <string_view>
#include <iostream>
#include <filesystem>
#include <thread>
#include <fstream>


struct State {
  std::string_view new_path;
  std::string project_name;
  bool isLib = false;
};

static inline State state;

int generate_base_dir(std::string path) {
  std::ofstream list(path.append("/CMakeLists.txt"));
  list << "cmake_minimum_required(VERSION 3.13)\n"
    << "project(" << state.project_name << ")\n"
    << "set(CMAKE_CXX_STANDARD 23)\n"
    << "set(CXX_STANDARD_REQUIRED YES)\n"
    << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n\n\n"
    << "add_compile_options(\n-Wall\n-pedantic\n)\n"
    << "add_subdirectory(src)\n";
  if(state.isLib) {
    list << "add_subdirectory(test)\n";
  }
  list << "include(FetchContent)\n";
  list.close();

  list.open(".gitinore");
  list << "build\n.cache\n";
  list.close();
  return 0;
}

int generate_src_dir(std::string path) {
  std::filesystem::create_directory(path);
  std::ofstream file(path + "CMakeLists.txt");

  std::cout << "Is a lib?: " << state.isLib << "\n";

  if(!state.isLib) {
    file << "set(SOURCE_FILES\n\t${CMAKE_SOURCE_DIR}/src/main.cc\n)\n"
      << "include_directories(${CMAKE_SOURCE_DIR}/src)\n"
      << "add_executable(${PROJECT_NAME} ${SOURCE_FILES})\n"
      << "target_link_libraries(${PROJECT_NAME})\n";

    file.close();
    file.open(path + ("main.cc"));
    file << "#include <iostream>\n\nauto main(int argc, char**argv) -> int {\n\tstd::cout << \"Hello, World!\\n\";\n\treturn 0;\n}\n";
    file.close();
  } else {
    file << "set(SOURCE_FILES\n\t${CMAKE_SOURCE_DIR}/src/" << state.project_name << ".cc\n\t${CMAKE_SOURCE_DIR}/src/" << state.project_name << ".h\n)\n"
      << "include_directories(${CMAKE_SOURCE_DIR}/src)\n"
      << "add_library(${PROJECT_NAME} ${SOURCE_FILES})\n"
      << "target_link_libraries(${PROJECT_NAME})\n";

    file.close();
    file.open(path + state.project_name + ".cc");
    file << "#include \"" << state.project_name << ".h\"\n\nnamespace " << state.project_name << " {\n\nauto add(int a, int b) -> int {\n\treturn a + b;\n}\n}\n";
    file.close();
    file.open(path + state.project_name + ".h");
    file << "#pragma once\n\nnamespace " << state.project_name << " {\nauto add(int a, int b) -> int;\n}\n";
    file.close();
  }
  return 0;
}

int generate_test_dir(std::string path) {
  std::filesystem::create_directory(path);
  std::ofstream list(path + ("CMakeLists.txt"));

  list << "include(FetchContent)\n"
    << "FetchContent_Declare(\n\tgoogletest\n\tURL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip\n)\n"
    << "set(gtest_force_shared_crt ON CACHE BOOL \"\" FORCE)\n"
    << "FetchContent_MakeAvailable(googletest)\n"
    << "enable_testing()\n"
    << "add_executable(base_test base.cc)\n"
    << "target_link_libraries(base_test GTest::gtest_main " << state.project_name << ")\n"
    << "include(GoogleTest)\n";
  list.close();
  list.open(path + "base.cc");
  list << "#include \"../src/" << state.project_name << ".h\"\n#include <gtest/gtest.h>\nusing namespace " << state.project_name << ";\n\nTEST(add, basic) {\n\tASSERT_EQ(4, add(3, 1));\n}"
;
  return 0;
}

int initFrateDir(std::string_view path) {
  std::cout << "[+] Initializing " << path << " as new C++ project\n";
  state.project_name = std::filesystem::path(path).filename().c_str();
  std::cout << state.project_name << "\n";
  std::string src = std::string(path) + "/src/";
  std::string test = std::string(path) + "/test/";

  std::thread t1(generate_base_dir, path.data());
  std::thread t2(generate_src_dir, src);
  if(state.isLib) {
    std::thread t3(generate_test_dir, test);
    t3.join();
  }

  t1.join();
  t2.join();
  return 0;
}

int newFrateDir(std::string_view pname) {

  std::string path = std::filesystem::current_path().append(std::filesystem::current_path().append(pname).string());
  std::cout << "[+] Creating new directory " << path << "\n";
  if(std::filesystem::exists(path)) {
    std::cout << "[-] Directory " << path << " already exists.\n";
    return -1;
  }
  std::filesystem::create_directory(path);
  initFrateDir(path);
  return 0;
}
void printUsage() {

  std::cerr << "Usage: \tfrate new [path]\n\tfrate init" << "\n";
}

int parse(const std::vector<std::string_view>& args) {

  if(args.size() < 1) {
    printUsage();
  }

  for(auto i = 0; i < args.size(); ++i) {
    if(args[i] == "new") {
      i++;
      if(args[i] == "--lib") {
        state.isLib = true;
        i++;
      }
      newFrateDir(std::filesystem::current_path().append(args[i]).c_str());
    } else if (args[i] == "init") {
      auto cur = std::filesystem::current_path().string();
      initFrateDir(cur);
    } else {
      printUsage();
    }

  }

  return 0;
}

int main(int argc, char **argv) {
  const static std::vector<std::string_view> args(argv + 1, argv + argc);
  parse(args);
  return 0;
}
