/***************************************************************
Copyright © wkangk <wangkangchn@163.com>
文件名		: main.cpp
作者	  	: wkangk <wangkangchn@163.com>
版本	   	: v1.0
描述	   	: 工厂模式
时间	   	: 2023-06-09 21:42
***************************************************************/
/* 
Factory 最重要的两个功能:
    (1) 定义创建对象的接口, 封装了对象的创建
    (2) 使得具体化类的工作延迟到了子类中
 */

#include <iostream>
#include <memory>


/* 产品基类 */
class Model
{
public:
    virtual ~Model() = default;
};

/* 空对象, 代替 nullptr */
class NullModel : public Model
{
public:
    NullModel() 
    {
        std::cout << "Null Model" << std::endl; 
    }
};

/* 该产品族的第一个产品分枝: 分类模型 */
class ClassificationModel : public Model
{
public:
    virtual ~ClassificationModel() = default;
};

/* 猫分类模型 */
class CatClassificationModel : public ClassificationModel
{
public:
    CatClassificationModel()
    {
        std::cout << "CatClassificationModel" << std::endl;
    }
};

/* 汽车分类模型 */
class CarClassificationModel : public ClassificationModel
{
public:
    CarClassificationModel()
    {
        std::cout << "CarClassificationModel" << std::endl;
    }
};


/* 该产品族的第二个产品分枝: 识别模型 */
class RecognitionModel : public Model
{
public:
    virtual ~RecognitionModel() = default;
};

/* 行人识别 */
class PersonRecognitionModel : public RecognitionModel
{
public:
    PersonRecognitionModel()
    {
        std::cout << "PersonRecognitionModel" << std::endl;
    }
};

/* 球识别 */
class BallRecognitionModel : public RecognitionModel
{
public:
    BallRecognitionModel()
    {
        std::cout << "BallRecognitionModel" << std::endl;
    }
};


/* 该产品族的第三个产品分枝: 特征计算模型 */
class FeatureModel : public Model
{
public:
    virtual ~FeatureModel() = default;
};

/* 步态特征 */
class GaitFeatureModel : public FeatureModel
{
public:
    GaitFeatureModel()
    {
        std::cout << "GaitFeatureModel" << std::endl;
    }
};

/* 人脸特征 */
class FaceFeatureModel : public FeatureModel
{
public:
    FaceFeatureModel()
    {
        std::cout << "FaceFeatureModel" << std::endl;
    }
};


enum ModelType
{
    CLASSIFICATION_CAT,
    CLASSIFICATION_CAR,

    RECOGNITION_PERSON,
    RECOGNITION_BALL,

    FEATURE_GAIT,
    FEATURE_FACE,
};


/* 模型工程 */
class ModelFactory
{
public:
    virtual ~ModelFactory() = default;
    virtual std::shared_ptr<Model> create_model(ModelType type) = 0;
};


class ClassificationModelFactory : public ModelFactory
{
public:
    std::shared_ptr<Model> create_model(ModelType type)
    {
        switch (type) {
            case CLASSIFICATION_CAT:
                return std::make_shared<CatClassificationModel>();

            case CLASSIFICATION_CAR:
                return std::make_shared<CarClassificationModel>();
            
            default:
                return std::make_shared<NullModel>();
        }
    }
};

class RecognitionModelFactory : public ModelFactory
{
public:
    std::shared_ptr<Model> create_model(ModelType type)
    {
        switch (type) {
            case RECOGNITION_PERSON:
                return std::make_shared<PersonRecognitionModel>();

            case RECOGNITION_BALL:
                return std::make_shared<BallRecognitionModel>();
            
            default:
                return std::make_shared<NullModel>();
        }
    }
};

class FeatureModelFactory : public ModelFactory
{
public:
    std::shared_ptr<Model> create_model(ModelType type)
    {
        switch (type) {
            case FEATURE_GAIT:
                return std::make_shared<GaitFeatureModel>();

            case FEATURE_FACE:
                return std::make_shared<FaceFeatureModel>();
            
            default:
                return std::make_shared<NullModel>();
        }
    }
};


int main()
{
    std::shared_ptr<ModelFactory> classification_model_factory(new ClassificationModelFactory);
    std::shared_ptr<ModelFactory> recognition_model_factory(new RecognitionModelFactory);
    std::shared_ptr<ModelFactory> feature_model_factory(new FeatureModelFactory);


    auto cat_classification_model = classification_model_factory->create_model(CLASSIFICATION_CAT); 
    auto car_classification_model = classification_model_factory->create_model(CLASSIFICATION_CAR); 
    auto null_model = classification_model_factory->create_model(FEATURE_FACE); 

    auto recognition_ball = recognition_model_factory->create_model(RECOGNITION_BALL);
    auto recognition_person = recognition_model_factory->create_model(RECOGNITION_PERSON);
    null_model = recognition_model_factory->create_model(FEATURE_FACE); 

    auto feature_face = feature_model_factory->create_model(FEATURE_FACE);
    auto feature_gait = feature_model_factory->create_model(FEATURE_GAIT);
    null_model = feature_model_factory->create_model(RECOGNITION_PERSON); 

    return 0;
}