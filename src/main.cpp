#include <iostream>
#include "ParserContext.hpp"
#include "Analysis.hpp"
#include "AnalysisVisitor.hpp"
#include "FileUtil.hpp"
#include "backend/CBackend.hpp"

namespace OpenABL {

void registerBuiltinFunctions(BuiltinFunctions &funcs) {
  funcs.add("dist", "dist_float2", { Type::VEC2, Type::VEC2 }, Type::FLOAT32);
  funcs.add("dist", "dist_float3", { Type::VEC3, Type::VEC3 }, Type::FLOAT32);
  funcs.add("near", "near",
      { { Type::ARRAY, Type::AGENT }, Type::AGENT, Type::FLOAT32 },
      { Type::ARRAY, Type::AGENT });
  funcs.add("save", "save", { { Type::ARRAY, Type::AGENT }, Type::STRING }, Type::VOID);
}

}

struct Options {
  std::string fileName;
  std::string backend;
  std::string outputDir;
};

static Options parseCliOptions(int argc, char **argv) {
  Options options = {};
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (i + 1 == argc) {
      std::cerr << "Missing argument for option \"" << arg << "\"" << std::endl;
      return {};
    }
    if (arg == "-b" || arg == "--backend") {
      options.backend = argv[++i];
    } else if (arg == "-i" || arg == "--input") {
      options.fileName = argv[++i];
    } else if (arg == "-o" || arg == "--output-dir") {
      options.outputDir = argv[++i];
    } else {
      std::cerr << "Unknown option \"" << arg << "\"" << std::endl;
      return {};
    }
  }

  if (options.fileName.empty()) {
    std::cerr << "Missing input file (-i or --input)" << std::endl;
    return {};
  }

  if (options.outputDir.empty()) {
    std::cerr << "Missing output directory (-o or --output-dir)" << std::endl;
    return {};
  }

  if (options.backend.empty()) {
    options.backend = "c";
  }

  return options;
}

int main(int argc, char **argv) {
  Options options = parseCliOptions(argc, argv);
  if (options.fileName.empty()) {
    return 1;
  }

  FILE *file = fopen(options.fileName.c_str(), "r");
  if (!file) {
    std::cerr << "File \"" << options.fileName << "\" could not be opened." << std::endl;
    return 1;
  }

  if (!OpenABL::createDirectory(options.outputDir)) {
    std::cerr << "Failed to create directory \"" << options.outputDir << "\"." << std::endl;
    return 1;
  }

  OpenABL::ParserContext ctx(file);
  if (!ctx.parse()) {
    return 1;
  }

  OpenABL::AST::Script &script = *ctx.script;

  OpenABL::ErrorStream err([](const OpenABL::Error &err) {
    std::cerr << err.msg << " on line " << err.loc.begin.line << std::endl;
  });

  OpenABL::BuiltinFunctions funcs;
  registerBuiltinFunctions(funcs);

  OpenABL::AnalysisVisitor visitor(err, funcs);
  script.accept(visitor);

  OpenABL::CPrinter printer;
  printer.print(script);
  OpenABL::writeToFile(options.outputDir + "/main.c", printer.extractStr());
  OpenABL::copyFile("asset/c/libabl.h", options.outputDir + "/libabl.h");
  OpenABL::copyFile("asset/c/build.sh", options.outputDir + "/build.sh");
  OpenABL::makeFileExecutable(options.outputDir + "/build.sh");

  fclose(file);
  return 0;
}
