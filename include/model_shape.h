#include "shape.h" 
#include "obj_loader.h" 
#include "state.h" 

#include <memory> 

class ModelShape : public Shape {
public:
    static ShapePtr Make(ImportedModel* model);
    ModelShape(ImportedModel* model);
    virtual ~ModelShape();
    
    void Draw(StatePtr state) override;

private:
    ImportedModel* m_model_ptr;
    
    ModelShape(const ModelShape&) = delete;
    ModelShape& operator=(const ModelShape&) = delete;
};