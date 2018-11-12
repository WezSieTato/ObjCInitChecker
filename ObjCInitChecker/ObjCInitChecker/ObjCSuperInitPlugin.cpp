#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;

namespace {

    class ObjCSuperInitVisitor : public RecursiveASTVisitor<ObjCSuperInitVisitor>
    {
    private:
        ASTContext *context;

    public:
        void setContext(ASTContext &context)
        {
            this->context = &context;
        }

        bool VisitObjCMethodDecl(ObjCMethodDecl *declaration)
        {
            if (!isInitImplementation(declaration)) {
                return true;
            }
            bool callInit = containsInit(declaration->getCompoundBody());
            if (!callInit) {
                reportMissingInit(declaration);
            }

            return true;
        }

    private:

        bool prefix(const std::string& a, const std::string& b) {
            if (a.size() > b.size()) {
                return a.substr(0,b.size()) == b;
            }
            else {
                return b.substr(0,a.size()) == a;
            }
        }

        bool isInit(std::string methodName) {
            return prefix(methodName, "init");
        }

        bool isInitImplementation(ObjCMethodDecl *declaration) {
            return isInit(declaration->getNameAsString()) && declaration->getCompoundBody();
        }

        bool containsInit(Stmt *statement) {
            if (statement->getStmtClass() == Stmt::ObjCMessageExprClass) {
                ObjCMessageExpr *objCExpr = (ObjCMessageExpr *)statement;
                if (isInit(objCExpr->getSelector().getAsString())) {
                    return true;
                }
            }

            for (auto child : statement->children()) {
                if (child && containsInit(child)) {
                    return  true;
                }
            }

            return false;
        }

        void reportMissingInit(ObjCMethodDecl *objCDecl) {
            DiagnosticsEngine &diagEngine = context->getDiagnostics();
            unsigned diagID = diagEngine.getCustomDiagID(DiagnosticsEngine::Error, "Missing init call");
            SourceLocation location = objCDecl->getCompoundBody()->getLocEnd();
            diagEngine.Report(location, diagID);

        }
    };

    class ObjCSuperInitConsumer : public ASTConsumer
    {
    public:
        void HandleTranslationUnit(ASTContext &context) {
            visitor.setContext(context);
            visitor.TraverseDecl(context.getTranslationUnitDecl());
        }
    private:
        ObjCSuperInitVisitor visitor;
    };

    class ObjCSuperInitAction : public PluginASTAction
    {
    public:
        virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) {
            return llvm::make_unique<ObjCSuperInitConsumer>();
        }

        bool ParseArgs(const CompilerInstance &CI, const
                       std::vector<std::string>& args) {
            return true;
        }
    };

}

static clang::FrontendPluginRegistry::Add<ObjCSuperInitAction>
X("ObjCSuperInitPlugin", "Check that init call [super init] or [self init]");
