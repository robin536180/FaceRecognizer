#include "DetectorSeeta.h"
#include "Log.h"
#include <QDir>
#include "Performance.h"

CDetectorSeeta::CDetectorSeeta(QObject *parent) 
    : CDetector(parent)
{}

CDetectorSeeta::~CDetectorSeeta()
{}

QVector<QRect> CDetectorSeeta::Detect(const QImage &image)
{
    PERFORMANCE(SeetaDectect)
    QImage img = image;
    if(img.format() != QImage::Format_RGB888)
    {
        PERFORMANCE_START(SeetaDectect)
        img = img.convertToFormat(QImage::Format_RGB888);     
        PERFORMANCE_ADD_TIME(SeetaDectect,
                             "conver format, image width:"
                             + QString::number(image.width())
                             + ";Height:"
                             + QString::number(image.height()))
    }
    img = img.rgbSwapped();
    
    SeetaImageData data;
    data.width = img.width();
    data.height = img.height();
    data.channels = 3;
    data.data = img.bits();
    
    QVector<QRect> facesRect;
    PERFORMANCE_START(SeetaDectect)
    SeetaFaceInfoArray faces = m_Dector->detect(data);
    PERFORMANCE_ADD_TIME(SeetaDectect, "detect")
    
    for(int i = 0; i < faces.size; i++)
    {
        QRect rect(faces.data[i].pos.x,
                   faces.data[i].pos.y,
                   faces.data[i].pos.width,
                   faces.data[i].pos.height);
        facesRect.push_back(rect);
    }
    PERFORMANCE_ADD_TIME(SeetaDectect, "copy reture value")
    return facesRect;
}

void CDetectorSeeta::UpdateParameter()
{
    if(!m_pParameter)
    {
        LOG_MODEL_ERROR("CDetectorSeeta", "The parameter is null");
        return;
    }
    
    seeta::ModelSetting::Device device = seeta::ModelSetting::CPU;
    switch (m_pParameter->GetDevice()) {
    case CParameter::CPU:
        device = seeta::ModelSetting::CPU;
        break;
    case CParameter::GPU:
        device = seeta::ModelSetting::GPU;
        break;
    default:
        LOG_MODEL_ERROR("CDetectorSeeta", "Don't support device %d",
                        m_pParameter->GetDevice());
        break;
    }
    
    int id = 0;
    
    QString szFile = m_pParameter->GetModelPath() + QDir::separator() + "fd_2_00.dat";
    seeta::ModelSetting model(szFile.toStdString(),
                              device,
                              id);
    m_Dector = QSharedPointer<seeta::FaceDetector>(new seeta::FaceDetector(model));
    m_Dector->set(seeta::FaceDetector::PROPERTY_MIN_FACE_SIZE, m_pParameter->GetMinFaceSize());
    return;
}
