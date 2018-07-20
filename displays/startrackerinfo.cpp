/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
#include "startrackerinfo.h"
#include "ui_startrackerinfo.h"
#include "projectMacros.h"

extern "C"
{
#include "linearAlgebra.h"
#include "rigidBodyKinematics.h"
    
#include "SpiceUsr.h"
}

StarTrackerInfo::StarTrackerInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::StarTrackerInfo),
    m_stLabel(new QLabel),
    m_FOVangle(8.0), // semicone angle [degrees]
    m_Nd(1),
    m_Nr(1),
    m_SRn(1),
    scene(new QGraphicsScene(this)),
    scene2(new QGraphicsScene(this)),
    m_width(250),
    m_height(250),
    m_DECn(0.0),
    m_DECs(0.0),
    m_RAe(0.0),
    m_RAw(0.0),
    m_iDECn(0),
    m_iDECs(0),
    m_iRAe(0),
    m_iRAw(0),
    m_upperBoundDEC(0),
    m_lowerBoundDEC(0),
    m_upperBoundRA(0),
    m_lowerBoundRA(0)
{
    ui->setupUi(this);
    addAction(ui->actionHead1);
    addAction(ui->actionHead2);
    this->setMaximumWidth(m_width);
    
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    ui->verticalLayout_2->setStretchFactor(ui->graphicsView, 1);
    ui->verticalLayout_2->setStretchFactor(ui->graphicsView2, 1);
    ui->graphicsView->setFixedHeight(m_height);
    ui->graphicsView2->setFixedHeight(m_height);
    
    QVBoxLayout *vBox = new QVBoxLayout;
    m_stLabel->setText("Star Tracker");
    m_stLabel->setMargin(6);
    m_stLabel->setAlignment(Qt::AlignCenter);
    vBox-> addWidget(m_stLabel);
    ui->GraphicGroup->setLayout(vBox);
    
    // ST SCENE VIEW
    int width = m_width;
    int height = m_height;
    
    // Scene head1
    scene->setSceneRect(QRect(0, 0, width, height));
    scene->setBackgroundBrush(Qt::black);
    ui->graphicsView ->setSceneRect(QRect(0, 0, width, height));
    ui->graphicsView ->setScene(scene);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setAlignment(Qt::AlignCenter);
    ui->graphicsView ->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView ->show();
    
    // Scene head2
    scene2->setSceneRect(QRect(0, 0, width, height));
    scene2->setBackgroundBrush(Qt::black);
    ui->graphicsView2 ->setSceneRect(QRect(0, 0, width, height));
    ui->graphicsView2 ->setScene(scene2);
    ui->graphicsView2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView2->setAlignment(Qt::AlignCenter);
    ui->graphicsView2 ->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView2 ->show();
    
    ui->InfoGroup->hide();
}

StarTrackerInfo::~StarTrackerInfo()
{
    delete ui;
}

void StarTrackerInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("stInfo");
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}
void StarTrackerInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("stInfo");
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void StarTrackerInfo::parseStarsData(const char *dataPath)
{
    FILE   *starDataFile;
    char starDataFileName[CHAR_BUFFER_SIZE];
    double n1, n2, n3;
    
    strcpy(starDataFileName, dataPath);
    strcat(starDataFileName, "inputStar.dat");
    
    std::cout << std::endl;
    starDataFile = fopen(starDataFileName, "r");
    if(starDataFile == NULL) {
        printf("Warning! initializeStarCatalog failed to open %s \n", starDataFileName);
        return;
    } else {
        printf("initializeStarCatalog succesfully opened! \n");
    }
    
    while(fscanf(starDataFile, "%lf %lf %lf", &n1, &n2, &n3)!= EOF)
    {
        RA.push_back(n1);
        DEC.push_back(n2);
        VM.push_back(n3);
    }
    
    m_SRn = ceil (0.5 * sqrt( m_FOVangle * m_FOVangle + m_FOVangle * m_FOVangle ));    // Spherical Rectangle Nominal Size
    m_Nd = int ( 180 / m_SRn );                              // DEC: [-90, 90]
    m_Nr = int ( 360 / m_SRn );                              // RA: [0, 360]
    
    /*Variables for debugging purposes*/
    std::cout << "SR nominal size: "<< m_SRn << std::endl;
    std::cout << "NDEC: "<< m_Nd << std::endl;
    std::cout << "NRA: "<< m_Nr << std::endl;
    int maxNumStarsSR = 0;
    int starsCounter = 0;
    int iDEC = 0;
    double Dfloor = -90.0;
    
    while (iDEC < m_Nd)
    {
        // Check how many stars are found in the DEC band at each iteration
        double Dceil = Dfloor + m_SRn;
        int counterDEC = 0;
        for (int i = 0; i < DEC.size(); i++)
        {
            if (DEC[i] >= Dfloor && DEC[i] < Dceil)
            {
                counterDEC += 1;
            }
        }
        // Create a vector that contains the indexs of the stars in the DEC band
        std::vector <int> i_vector(counterDEC);
        int i_local = 0;
        for (int i = 0; i < DEC.size(); i++)
        {
            if (DEC[i] >= Dfloor && DEC[i] < Dceil)
            {
                i_vector[i_local] = i;
                i_local ++;
            }
        }
        
        // Loop through all the Sphere Rectangles contained in a DEC range at each iteration
        int iRA = 0;
        double Rfloor = 0.0;
        while (iRA < m_Nr)
        {
            // Check how many stars there are in the SR
            double Rceil = Rfloor + m_SRn;
            int counterRA = 0;
            for (int j = 0; j < counterDEC; j++)
            {
                if (RA[i_vector[j]] >= Rfloor && RA[i_vector[j]] < Rceil)
                {
                    counterRA += 1;
                }
            }
            // Create a vector that contains the index of the stars in the SR
            std::vector <int> j_vector(counterRA);
            int j_local = 0;
            for (int j = 0; j < counterDEC; j++)
            {
                if (RA[i_vector[j]] >= Rfloor && RA[i_vector[j]] < Rceil)
                {
                    j_vector[j_local] = i_vector[j];
                    j_local ++;
                }
            }
            
            starsCounter += int(j_vector.size());
            if (j_vector.size()> maxNumStarsSR){ maxNumStarsSR = int(j_vector.size()); }
            
            // Save the information of the stars in each SR in a matrix
            matrix SR_m(3, std::vector<double>(j_vector.size()));
            int k = 0;
            for (k = 0; k < j_vector.size(); k++)
            {
                SR_m[0][k] = DEC[j_vector[k]];
                SR_m[1][k] = RA[j_vector[k]];
                SR_m[2][k] = VM[j_vector[k]];
            }
            bandRA.push_back(SR_m);
            
            iRA += 1;
            Rfloor += m_SRn;
        }
        bandDEC.push_back(bandRA);
        bandRA.resize(0);
        iDEC += 1;
        Dfloor += m_SRn;
    }
    std::cout << "Total number of stars stored: "<< starsCounter << std::endl;
    std::cout << "Maximum number of stars in a Sphere Rectangle: "<< maxNumStarsSR << std::endl;
    
    if(fclose(starDataFile) != 0) {
        printf("Warning! initializeStarCatalog failed to close %s \n", starDataFileName);
    } else {
        printf("initializeStarCatalog succesfully closed! \n");
    }
    std::cout << std::endl;
}

void StarTrackerInfo::updateST(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager= dynamic_cast <AdcsSimDataManager*>(simDataManager);
    SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
    
    m_stLabel->setStyleSheet(getStateStyleSheet(scSim->st.state, scSim->st.resetCounter));
    m_stLabel->setToolTip(getStateToolTip(scSim->st.state, scSim->st.resetCounter));
    
    
    // Star Field View
    double BN[3][3];            // DCM from inertial to body frame
    double NB[3][3];            // DCM from body frame to inertial frame
    MRP2C(scSim->sigma, BN);
    m33Transpose(BN, NB);
    double inertialCelestialBodytoJ2000[3][3];
    CelestialObject_t celestBody = scSim->celestialObject;
    switch(celestBody) {
        case CELESTIAL_EARTH:
            break;
        case CELESTIAL_MARS:
            pxform_c("MARSIAU", "J2000", scSim->ics.ET0 + scSim->time, inertialCelestialBodytoJ2000);
            m33MultM33(inertialCelestialBodytoJ2000, NB, NB);
            break;
        default:
            printf("WARNING: un-implemented celestial object in Star Tracker View.\n");
            break;
    }
    double z_scHat[3]; // sc Z-axis vector in J2000 frame
    double z_sc[3] = {0, 0, 1};
    m33MultV3(NB, z_sc, z_scHat);
    v3Normalize(z_scHat, z_scHat);
    
    double z_J2000[3];
    v3Set(0, 0, 1, z_J2000);
    v3Normalize(z_J2000, z_J2000);
    
    double cosinusEps = v3Dot(z_scHat, z_J2000);
    double Eps = acos(cosinusEps) * R2D;
    
    ui->graphicsView->resetTransform();
    ui->graphicsView->resetMatrix();
    ui->graphicsView->rotate(-Eps);
    
    ui->graphicsView2->resetTransform();
    ui->graphicsView2->resetMatrix();
    ui->graphicsView2->rotate(-Eps);
    
    double stHat1[3]; // ST head 1 pointing vector in J2000 frame
    double stHat2[3]; // ST head 2 pointing vector in J2000 frame
    m33MultV3(NB, scSim->st.gs_head1, stHat1);
    m33MultV3(NB, scSim->st.gs_head2, stHat2);
    v3Normalize(stHat1, stHat1);
    v3Normalize(stHat2, stHat2);
    
    double r = 1.0; // module of vector
    double RA_head1;
    double DEC_head1;
    double RA_head2;
    double DEC_head2;
    recrad_c(stHat1, &r, &RA_head1, &DEC_head1);
    recrad_c(stHat2, &r, &RA_head2, &DEC_head2);
    
    RA_head1 *= R2D;
    DEC_head1 *= R2D;
    RA_head2 *= R2D;
    DEC_head2 *= R2D;
    
    // HEAD 1
    // star tracker bore sight bounds
    m_DECn = DEC_head1 + m_SRn;
    if (m_DECn > 90){ m_DECn = 90; }
    m_DECs = DEC_head1 - m_SRn;
    if (m_DECs < -90){ m_DECs = -90; }
    m_RAe = RA_head1 + m_SRn / cos(DEC_head1*D2R);
    if (m_RAe > 360){ m_RAe = 360; }
    m_RAw = RA_head1 - m_SRn / cos(DEC_head1*D2R);
    if (m_RAw < 0){ m_RAw = 0; }
    
    m_iDECn = int((m_DECn + 90) / m_SRn + 0.5); // Note: 0.5 is added to round to the nearest int
    m_iDECs = int((m_DECs + 90) / m_SRn + 0.5);
    m_iRAe = int(m_RAe / m_SRn + 0.5);
    m_iRAw = int(m_RAw / m_SRn + 0.5);
    
    scene->clear();
    int numStars = 0;
    if( bandDEC.size() >= m_iDECn)
    {
        if ( bandDEC[m_iDECn].size() >= m_iRAe)
        {
            for(int i = m_iDECs; i < m_iDECn; i++)
            {
                for(int j = m_iRAw; j < m_iRAe; j++)
                {
                    numStars += bandDEC[i][j][0].size();
                    for (int k=0; k<bandDEC[i][j][0].size(); k++)
                    {
                        drawStar(bandDEC[i][j][0][k], bandDEC[i][j][1][k], bandDEC[i][j][2][k], scene);
                    }
                }
            }
        }
    }
    
    // HEAD 2
    // star tracker bore sight bounds
    m_DECn = DEC_head2 + m_SRn;
    if (m_DECn > 90){ m_DECn = 90; }
    m_DECs = DEC_head2 - m_SRn;
    if (m_DECs < -90){ m_DECs = -90; }
    m_RAe = RA_head2 + m_SRn / cos(DEC_head2*D2R);
    if (m_RAe > 360){ m_RAe = 360; }
    m_RAw = RA_head2 - m_SRn / cos(DEC_head2*D2R);
    if (m_RAw < 0){ m_RAw = 0; }
    
    m_iDECn = int((m_DECn + 90) / m_SRn + 0.5); // Note: 0.5 is added to round to the nearest int
    m_iDECs = int((m_DECs + 90) / m_SRn + 0.5);
    m_iRAe = int(m_RAe / m_SRn + 0.5);
    m_iRAw = int(m_RAw / m_SRn + 0.5);
    
    scene2->clear();
    if( bandDEC.size() >= m_iDECn)
    {
        if ( bandDEC[m_iDECn].size() >= m_iRAe)
        {
            for(int i = m_iDECs; i < m_iDECn; i++)
            {
                for(int j = m_iRAw; j < m_iRAe; j++)
                {
                    for (int k=0; k<bandDEC[i][j][0].size(); k++)
                    {
                        drawStar(bandDEC[i][j][0][k], bandDEC[i][j][1][k], bandDEC[i][j][2][k], scene2);
                    }
                }
            }
        }
    }
    
}

void StarTrackerInfo::drawStar(double dec, double ra, double vm, QGraphicsScene *sceneH)
{
    double rad = 5;
    int mag = 105;
    int power = (255-mag)/5;
    
    /* A logarithmic scale of 2.512 between magnitude levels is implemented
     for star's size (rad), simulating differences in brightness:
     http://www.stargazing.net/david/constel/magnitude.html
     Special cases are vm = 0, vm = 1, vm = 7, for convenience */
    switch(int(vm))
    {
        case 0:
            mag += power*5;
            rad = 2.4;
            break;
        case 1:
            mag += power*5;
            rad = 2.4;
            break;
        case 2:
            mag += power*4;
            rad *= 0.398;
            break;
        case 3:
            mag += power*3;
            rad *= 0.158;
            break;
        case 4:
            mag += power*2;
            rad *= 0.063;
            break;
        case 5:
            mag += power;
            rad *=0.251;
            break;
        case 6:
            rad *= 0.1;
            break;
        default:
            rad *= 0.1;
            break;
    }
    rad += 0.1;
    QColor starColor = qRgb(mag, mag, mag);
    
    double y = (m_DECn - dec) / (m_DECn - m_DECs) * m_height;
    double x = (ra - m_RAw) / (m_RAe - m_RAw) * m_width;
    
    if (dec >= m_DECs && dec <= m_DECn)
    {
        if (ra >= m_RAw && ra <= m_RAe)
        {
            sceneH->addEllipse(x, y, rad, rad, QPen(starColor), QBrush(starColor));
        }
    }
}

QString StarTrackerInfo::getStateStyleSheet(ComponentState_t componentState, int resetCounter)
{
    QString output = "QLabel{color:black;";
    switch(componentState) {
        case COMPONENT_OFF:
            output.append(" background-color:darkslategray;");
            break;
        case COMPONENT_START:
            output.append(" background-color:greenyellow;");
            break;
        case COMPONENT_ON:
            output.append(" background-color:green;");
            break;
        case COMPONENT_FAULT:
            output.append(" background-color:red;");
            break;
        case COMPONENT_TEST:
            output.append(" background-color:orange;");
            break;
        default:
            output.append(" background-color:white;");
            break;
    }
    if(resetCounter == 0) {
        output.append("}");
    } else {
        output.append(" border-color:red; border-width:0.5px; border-style:solid;}");
    }
    return output;
}

QString StarTrackerInfo::getStateToolTip(ComponentState_t componentState, int resetCounter)
{
    QString output;
    switch(componentState) {
        case COMPONENT_OFF:
            output.append("Off");
            break;
        case COMPONENT_START:
            output.append("Start");
            break;
        case COMPONENT_ON:
            output.append("On");
            break;
        case COMPONENT_FAULT:
            output.append("Fault");
            break;
        case COMPONENT_TEST:
            output.append("Test");
            break;
        default:
            output.append("Unknown");
            break;
    }
    if(resetCounter > 0) {
        output.append(" (");
        output.append(QString::number(resetCounter));
        output.append(" resets)");
    }
    return output;
}