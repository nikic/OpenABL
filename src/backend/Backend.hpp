#pragma once

#include "AST.hpp"

namespace OpenABL {

struct Backend {
  virtual void generate(AST::Script &script, const std::string &targetDir,
                        const std::string &assetDir) = 0;
};

}
