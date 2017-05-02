#include <sstream>
#include "Backend.hpp"
#include "FileUtil.hpp"
#include "XmlUtil.hpp"
#include "FlameModel.hpp"
#include "FlameGPUPrinter.hpp"

namespace OpenABL {

static XmlElems createXmlAgents(AST::Script &script, const FlameModel &model) {
  XmlElems xagents;
  for (const AST::AgentDeclaration *decl : script.agents) {
    XmlElems members;
    auto unpackedMembers = FlameModel::getUnpackedMembers(*decl->members);
    for (const FlameModel::Member &member : unpackedMembers) {
      members.push_back({ "gpu:variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    XmlElems functions;
    for (const FlameModel::Func &func : model.funcs) {
      if (func.agent != decl) {
        continue;
      }

      XmlElems inputs, outputs;

      if (!func.inMsgName.empty()) {
        inputs.push_back({ "gpu:input", {
          { "messageName", {{ func.inMsgName }} },
        }});
      }

      if (!func.outMsgName.empty()) {
        outputs.push_back({ "gpu:output", {
          { "messageName", {{ func.outMsgName }} },
          { "gpu:type", {{ "single_message" }} },
        }});
      }

      functions.push_back({ "gpu:function", {
        { "name", {{ func.name }} },
        { "currentState", {{ "default" }} },
        { "nextState", {{ "default" }} },
        { "inputs", inputs },
        { "outputs", outputs },
        { "gpu:reallocate", {{ "false" }} },
        { "gpu:RNG", {{ "false" }} },
      }});
    }

    xagents.push_back({ "gpu:xagent", {
      { "name", {{ decl->name }} },
      { "memory", members },
      { "functions", functions },
    }});
  }
  return xagents;
}

static XmlElems createXmlMessages(const FlameModel &model) {
  XmlElems messages;
  for (const FlameModel::Message &msg : model.messages) {
    XmlElems variables;
    auto unpackedMembers = FlameModel::getUnpackedMembers(msg.members);
    for (const FlameModel::Member &member : unpackedMembers) {
      variables.push_back({ "gpu:variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    messages.push_back({ "gpu:message", {
      { "name", {{ msg.name }} },
      { "variables", variables },
    }});
  }
  return messages;
}

static XmlElems createXmlLayers(const FlameModel &model) {
  XmlElems layers;
  for (const FlameModel::Func &func : model.funcs) {
    layers.push_back({ "layer", {
      { "gpu:layerFunction", {
        { "name", {{ func.name }} },
      }},
    }});
  }
  return layers;
}

static std::string createXmlModel(AST::Script &script, const FlameModel &model) {
  XmlElems xagents = createXmlAgents(script, model);
  XmlElems messages = createXmlMessages(model);
  XmlElems layers = createXmlLayers(model);
  XmlElem root("gpu:model", {
    { "name", {{ "TODO" }} },
    { "gpu:environment", {
      { "gpu:functionFiles", {
        { "file", {{ "functions.c" }} },
      }}
    }},
    { "xagents", xagents },
    { "messages", messages },
    { "layers", layers },
  });
  root.setAttr("xmlns:gpu", "http://www.dcs.shef.ac.uk/~paul/XMMLGPU");
  root.setAttr("xmlns", "http://www.dcs.shef.ac.uk/~paul/XMML");
  XmlWriter writer;
  return writer.serialize(root);
}

static std::string createFunctionsFile(AST::Script &script, const FlameModel &model) {
  FlameGPUPrinter printer(script, model);
  printer.print(script);
  return printer.extractStr();
}

void FlameGPUBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  (void) assetDir;

  FlameModel model = FlameModel::generateFromScript(script);

  createDirectory(outputDir + "/model");
  createDirectory(outputDir + "/dynamic");

  writeToFile(outputDir + "/model/XMLModelFile.xml", createXmlModel(script, model));
  writeToFile(outputDir + "/model/functions.c", createFunctionsFile(script, model));
}

}
