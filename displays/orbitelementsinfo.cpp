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
#include "orbitelementsinfo.h"
#include "ui_orbitelementsinfo.h"
extern "C" {
#include "linearAlgebra.h"
#include "orbitalMotion.h"
}
#include <QAction>
#include <QMenu>

OrbitElementsInfo::OrbitElementsInfo(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::OrbitElementsInfo),
    b(0.0),
    apoapsis(0.0),
    periapsis(0.0),
    M(0.0),
    E(0.0),
    B(0.0),
    m_orbit(new Orbit2D(this)),
    m_fraction(new OrbitalPeriod(this)),
    m_Tlabel(new QLabel(this)),
    m_counter(0)

{
    ui->setupUi(this);
    this->setMaximumWidth(210);
//    ui->VerboseGroupNew->setContextMenuPolicy(Qt::CustomContextMenu);
//    connect(ui->VerboseGroupNew,SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
    
    //addAction(ui->actionVerbose_DisplayNew);
    //addAction(ui->actionGraphic_Display);
    ui->actionVerbose_DisplayNew->setChecked(true);
    ui->actionGraphic_Display->setChecked(true);
    ui->verticalLayout_3->addWidget(m_orbit);
    
    QVBoxLayout *box = new QVBoxLayout;
    m_Tlabel->setAlignment(Qt::AlignCenter);
    box->addWidget(m_Tlabel);
//    box->addWidget(m_fraction);
    ui->verticalLayout_3->addLayout(box);
}

OrbitElementsInfo::~OrbitElementsInfo()
{
    delete ui;
}

void OrbitElementsInfo::showContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu(ui->VerboseGroupNew);
    QAction *actionAU = new QAction(tr("Display units in AU"), menu);
    QAction *actionKm = new QAction(tr("Display units in km"), menu);
    menu->addAction(actionAU);
    //connect(actionLevel, SIGNAL(triggered()), acsList1[i], SLOT(show()));
    menu->addAction(actionKm);
    //connect(actionLevelHide, SIGNAL(triggered()), acsList1[i], SLOT(hide()));
    menu->exec(ui->VerboseGroupNew->mapToGlobal(pos));
}

void OrbitElementsInfo::saveSettings(QSettings *settings)
{
    settings->beginGroup("orbitElementsInfo");
    settings->setValue("viewVerbose",ui->actionVerbose_DisplayNew->isChecked());
    settings->setValue("viewGraphic",ui->actionGraphic_Display->isChecked());
    settings->setValue("geometry", saveGeometry());
    settings->setValue("floating", isFloating());
    settings->setValue("pos", pos());
    settings->setValue("size", size());
    settings->endGroup();
}

void OrbitElementsInfo::loadSettings(QSettings *settings)
{
    settings->beginGroup("orbitElementsInfo");
    ui->actionVerbose_DisplayNew->setChecked(settings->value("viewVerbose").toBool());
    ui->actionGraphic_Display->setChecked(settings->value("viewGraphic").toBool());
    restoreGeometry(settings->value("geometry").toByteArray());
    setFloating(settings->value("floating").toBool());
    move(settings->value("pos", pos()).toPoint());
    resize(settings->value("size", size()).toSize());
    settings->endGroup();
}

void OrbitElementsInfo::updateOrbitElements(AdcsSimDataManager *simDataManager)
{
    AdcsSimDataManager *adcsSimDataManager = dynamic_cast<AdcsSimDataManager *>(simDataManager);
    if (adcsSimDataManager)
    {
        SpacecraftSim *scSim = adcsSimDataManager->getSpacecraftSim();
        
        // Update Display
        char outString[256]=" ";
 
        ui->a1Label->setToolTip("Semimajor Axis");
        ui->a1Label->setToolTipDuration(2000);
        ui->a3Label->setText("km");
        sprintf(outString, "%6.0f", scSim->oe.a);
        if (scSim->celestialObject== CELESTIAL_SUN)
        {
            sprintf(outString, "%6.4f", scSim->oe.a * 1./AU);
            ui->a3Label->setText("AU");
        }
        ui->a2Label->setText(outString);
        ui->e1Label->setToolTip("Eccentricity");
        ui->e1Label->setToolTipDuration(2000);
        sprintf(outString, "%6.4f", scSim->oe.e);
        ui->e2Label->setText(outString);
        ui->i1Label->setToolTip("Inclination");
        ui->i1Label->setToolTipDuration(2000);
        sprintf(outString, "%+.1f",(scSim->oe.i)*R2D);//deg
        ui->i3Label->setText("deg");
        ui->i2Label->setText(outString);
        ui->an1Label->setText(QChar(0xA9,0x03));
        ui->an1Label->setToolTip("Ascending Node");
        ui->an1Label->setToolTipDuration(2000);
        sprintf(outString, "%+.2f", scSim->oe.Omega);
        ui->an2Label->setText(outString);
        ui->an3Label->setToolTip("hours since midnight");
        ui->an3Label->setToolTipDuration(2000);
        ui->p1Label->setText(QChar(0xC9,0x03));
        ui->p1Label->setToolTip("Argument of Perigee");
        ui->p1Label->setToolTipDuration(2000);
        sprintf(outString, "%+.2f",(scSim->oe.omega))*R2D; //deg
        ui->p3Label->setText("deg");
        ui->p2Label->setText(outString);
        ui->tA1Label->setText("f");
        ui->tA1Label->setToolTip("True Anomaly");
        ui->tA1Label->setToolTipDuration(2000);
        sprintf(outString, "%+.1f", (scSim->oe.f)*R2D); //deg
        ui->tA3Label->setText("deg");
        ui->tA2Label->setText(outString);
        ui->T01Label->setToolTip("Mean Anomaly at Epoch");
        ui->T01Label->setToolTipDuration(2000);
        sprintf(outString, "%+6.1f", (scSim->ics.M0));
        ui->T02Label->setText(outString);
        

        if (scSim->oe.e>=0 && scSim->oe.e<1)
        {
            b = (scSim->oe.a)*sqrt(1-(scSim->oe.e)*(scSim->oe.e));
            apoapsis = (scSim->oe.a)*(scSim->oe.e + 1);
            periapsis = (scSim->oe.a)*(1 - scSim->oe.e);
            E = f2E(scSim->oe.f, scSim->oe.e);
            M = E2M(E, scSim->oe.e);
            
            ui->aa1Label->setToolTip("Apoapsis");
            ui->aa1Label->setToolTipDuration(2000);
            ui->aa3Label->setText("km");
            sprintf(outString, "%6.0f", this->apoapsis);
            if (scSim->celestialObject== CELESTIAL_SUN)
            {
                sprintf(outString, "%6.4f", this->apoapsis * 1./AU);
                ui->aa3Label->setText("AU");
            }
            ui->aa2Label->setText(outString);
            ui->mA1Label->setToolTip("Mean Anomaly");
            ui->mA1Label->setToolTipDuration(2000);
            sprintf(outString, "%6.1f", (this->M));
            ui->mA2Label->setText(outString);
            ui->eA1Label->setToolTip("Eccentric Anomaly");
            ui->eA1Label->setToolTipDuration(2000);
            sprintf(outString, "%+5.1f", (this->E)*R2D); //deg
            ui->eA3Label->setText("deg");
            ui->eA2Label->setText(outString);

        }
        else if (scSim->oe.e>1)
        {
            B= acos(1/(scSim->oe.e));
            b=(scSim->oe.a)*(scSim->oe.e)*sin(B);
            periapsis = (scSim->oe.a)*(scSim->oe.e-1);
            E = f2H(scSim->oe.f, scSim->oe.e);
            M = H2N(E,scSim->oe.e);
            
            ui->aa1Label->setText(QChar(0xB4,0x03));
            ui->aa1Label->setToolTip("Hyperbolic Deflection Angle");
            ui->aa1Label->setToolTipDuration(2000);
            sprintf(outString, "%6.1f", (M_PI-2*(this->B))*R2D);//deg
            ui->aa2Label->setText(outString);
            ui->aa3Label->setText("deg");
            
            ui->mA1Label->setText("N");
            ui->mA1Label->setToolTip("Mean Hyperbolic Anomaly");
            ui->mA1Label->setToolTipDuration(2000);
            sprintf(outString, "%6.5f", (this->M));
            ui->mA2Label->setText(outString);
            ui->eA1Label->setText("H");
            ui->eA1Label->setToolTip("Hyperbolic Anomaly");
            ui->eA1Label->setToolTipDuration(2000);
            sprintf(outString, "%+5.1f", (this->E)*R2D); //deg
            ui->eA3Label->setText("deg");
            ui->eA2Label->setText(outString);
        };
        
        ui->pa1Label->setToolTip("Periapsis");
        ui->pa1Label->setToolTipDuration(2000);
        ui->pa2Label1->setText("km");
        sprintf(outString, "%6.0f", this->periapsis);
        if (scSim->celestialObject== CELESTIAL_SUN)
        {
            sprintf(outString, "%6.4f", this->periapsis * 1./AU);
            ui->pa2Label1->setText("AU");
        }
        ui->pa2Label->setText(outString);
        ui->sma1Label->setToolTip("Semiminor Axis");
        ui->sma1Label->setToolTipDuration(2000);
        ui->sma2Label1->setText("km");
        sprintf(outString, "%6.0f", this->b);
        if (scSim->celestialObject== CELESTIAL_SUN)
        {
            sprintf(outString, "%6.4f", this->b * 1./AU);
            ui->sma2Label1->setText("AU");
        }
        ui->sma2Label->setText(outString);
        
        // Not sure if we should include these parameters
        ui->mA1Label->hide();
        ui->mA2Label->hide();
        ui->eA1Label->hide();
        ui->eA2Label->hide();
        ui->eA3Label->hide();
        ui->T01Label->hide();
        ui->T02Label->hide();
        ui->T03Label->hide();
        
        // Draw Graphic
        double ref = 1;
        switch(scSim->celestialObject) {
            case CELESTIAL_EARTH:
                ref = REQ_EARTH;
                break;
            case CELESTIAL_MARS:
                ref = REQ_MARS;
                break;
            case CELESTIAL_SUN:
                ref = REQ_SUN;
            default:
                ref = 1;
                break;
        }
        m_orbit->getOrbit(scSim->oe.e, scSim->oe.a, scSim->oe.f, ref);
        
        char Tstring[256]="";
        double T = sqrt(4 * M_PI * M_PI * fabs(scSim->oe.a * scSim->oe.a * scSim->oe.a) / scSim->mu) / 3600; // [hours]
        T = T / 365; // [days]
        sprintf(Tstring, "%.2f", T);
        char Mstring[256]="";
        double fraction = M / (2 * M_PI);
        if (M<0)
        {
            fraction = 1 - fabs(fraction);
        }
        sprintf(Mstring, "%.2f", fraction);
//        m_Tlabel->setText("<font size=2>Orbital Period</font><br><font color='yellow' size=2>T = "+ QString(Tstring)+" h</font><br><br><font size=2>T Fraction since Perigee </font><br><font color='yellow' size=2>p = "+QString(Mstring)+"</font>");
        m_Tlabel->setText("<font size=2>Orbital Period</font><br><font color='yellow' size=2>T = "+ QString(Tstring)+" days</font>");
        double fraction0 = scSim->ics.M0 / (2 * M_PI);
        if (scSim->ics.M0<0)
        {
            fraction0 = 1 - fabs(fraction0);
        }
        
        m_counter = floor(int(scSim->time / (T * 3600)));
        if (scSim->time>0)
        {
            m_fraction->getOrbitFraction(fraction, fraction0, m_counter);
        }
        if (scSim->oe.a < 0 /*|| scSim->adcsState == ADCS_THRUSTING*/)
        {
            m_fraction->hide();
            m_Tlabel->hide();
        } else if (scSim->adcsState == ADCS_THRUSTING) {
            m_fraction->hide();
            m_Tlabel->show();
        } else {
            m_Tlabel->show();
        }
    }
}