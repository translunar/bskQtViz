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
#include "adcssimdatamanager.h"
#include "utilities/astroConstants.h"
#include "linestrip.h"
#include "graphics.h"
#include <sstream>
#include <chrono>


extern "C" {
#include "utilities/rigidBodyKinematics.h"
#include "utilities/orbitalMotion.h"
#include "SpiceUsr.h"
#include "utilities/linearAlgebra.h"
}

AdcsSimDataManager::AdcsSimDataManager(QObject *parent)
    : SimDataManager(parent)
    , m_client()
    , m_isConnected(false)
{
    this->receivedNewData = false;
    m_toggleableObjects.insert("Spacecraft Body Frame Axes", true);
    m_toggleableObjects.insert("Spacecraft Hill Frame Axes", true);
    m_toggleableObjects.insert("Spacecraft Velocity Frame Axes", true);
    m_toggleableObjects.insert("Spacecraft Orbit", true);
    m_toggleableObjects.insert("Spacecraft Sun-Direction Vector", false);
    //m_toggleableObjects.insert("Spacecraft Earth-Direction Vector", true);
    //m_toggleableObjects.insert("Spacecraft Magnetic Field Vector", false);
    m_toggleableObjects.insert("Celestial Object Orbits", true);
    //m_toggleableObjects.insert("Planet-Centered Fixed Axes", true);
    //m_toggleableObjects.insert("Planet-Centered Inertial Axes", true);
    //m_toggleableObjects.insert("CSS Photo Diode Normals", false);
    //m_toggleableObjects.insert("CSS Field Of View", false);
    m_toggleableObjects.insert("Reaction Wheel Pyramid",false);
    //m_toggleableObjects.insert("Torque Rod Pyramid",false);
    //m_toggleableObjects.insert("Star Tracker Pointing Normals",false);
    //m_toggleableObjects.insert("Star Tracker Field of View",false);
}

AdcsSimDataManager::~AdcsSimDataManager()
{
    
}

bool AdcsSimDataManager::openConnection(QString ipAddress, QString port)
{
    switch(m_inputType) {
        case INPUT_CONNECTION:
            closeConnection();
            break;
        case INPUT_FILE:
            closeFile();
            break;
        case INPUT_NONE:
            break;
    }
    
    if(m_client.connect(ipAddress.toStdString(), port.toStdString())) {
        updateSimObjects();
        // Start timer to keep checking for more data
        m_connectionTimerId = QObject::startTimer(30);
        m_inputType = INPUT_CONNECTION;
    } else {
        emit showMessage("Connection attempt failed", 5000);
        m_inputType = INPUT_NONE;
    }
    return true;
}

bool AdcsSimDataManager::closeConnection()
{
    emit showMessage("Closing connection...", 2000);
    if(!m_client.close()) {
        std::cout << "called by " << __FUNCTION__ << std::endl;
        return false;
    }
    m_isConnected = false;
    this->m_scSim.timeStamp = 0;
    this->receivedNewData = false;
    killTimer(m_connectionTimerId);
    m_inputType = INPUT_NONE;
    return true;
}

bool AdcsSimDataManager::openFile(QString filename)
{
    switch(m_inputType) {
        case INPUT_CONNECTION:
            closeConnection();
            break;
        case INPUT_FILE:
            closeFile();
            break;
        case INPUT_NONE:
            break;
    }
    
    // TODO: Add code to actually open file
    
    updateSimObjects();
    
    m_inputType = INPUT_FILE;
    return true;
}

bool AdcsSimDataManager::closeFile()
{
    // TODO: Add code to actually close file
    
    m_inputType = INPUT_NONE;
    return true;
}

QVector3D AdcsSimDataManager::getLightPosition()
{
    if (m_scSim.celestialObject != CELESTIAL_SUN) {
        return QVector3D(m_scSim.sHatN[0], m_scSim.sHatN[1], m_scSim.sHatN[2]).normalized() * (float)AU;
    } else {
        return QVector3D(0., 0., 0.);
    }
}

void AdcsSimDataManager::setSimTime(double time)
{
    if(m_inputType == INPUT_FILE) {
        // TODO: Do something here
    }
}

//void AdcsSimDataManager::setNewManeuver(double start, double duration)
//{
//    m_scSimVisualization.thrusterStartTime = start;
//    m_scSimVisualization.thrusterDurationTime = duration;
//}
//void AdcsSimDataManager::setStateTAM(ComponentState_t state)
//{
//    m_scSimVisualization.stateTAM = state;
//}
//void AdcsSimDataManager::setStateIRU(ComponentState_t state)
//{
//    m_scSimVisualization.stateIRU = state;
//}
//void AdcsSimDataManager::setStatePOD(ComponentState_t state)
//{
//    m_scSimVisualization.statePOD = state;
//}
//void AdcsSimDataManager::setOnOffState(ComponentState_t state, int index)
//{
//    m_scSimVisualization.onOffState = state;
//    m_scSimVisualization.index = index;
//}
//void AdcsSimDataManager::setStateTR(ComponentState_t state, int index)
//{
//    m_scSimVisualization.stateTR = state;
//    m_scSimVisualization.indexTR = index;
//}
//void AdcsSimDataManager::setStateCSS(ComponentState_t state, int index)
//{
//    m_scSimVisualization.stateCSS = state;
//    m_scSimVisualization.indexCSS = index;
//}
//void AdcsSimDataManager::setGravityGradientTorque(int mode)
//{
//    m_scSimVisualization.useGravityGradientTorque= mode;
//}
//void AdcsSimDataManager::setReactionWheelJitter(int mode)
//{
//    m_scSimVisualization.useReactionWheelJitter = mode;
//}
//void AdcsSimDataManager::setGravityPerturbModel(int mode)
//{
//    m_scSimVisualization.useGravityPerturbModel = mode;
//}
//void AdcsSimDataManager::setAtmosDragModel(int mode)
//{
//    m_scSimVisualization.useAtmosDragModel = mode;
//}
//void AdcsSimDataManager::setSrpModel(int mode)
//{
//    m_scSimVisualization.useSrpModel = mode;
//}
//void AdcsSimDataManager:: setAlbedoModel(AlbedoModel_t model)
//{
//    m_scSimVisualization.albedoModel = model;
//}
//void AdcsSimDataManager:: setEarthMagField(EarthMagFieldModel_t model)
//{
//    m_scSimVisualization.earthMagFieldModel = model;
//}

//void AdcsSimDataManager::perturbRates()
//{
//    m_scSimVisualization.perturbRates = true;
//}
//
//void AdcsSimDataManager::setControlState(CommandedState_t state)
//{
//    m_scSimVisualization.controlState = state;
//}
//
//void AdcsSimDataManager::setAdcsState(ADCSState_t state)
//{
//    m_scSimVisualization.adcsState = state;
//}
//
//void AdcsSimDataManager::setThrustingControlMode(int value)
//{
//    m_scSimVisualization.controlUsingThrusters = value;
//}
//
void AdcsSimDataManager::setPlaySpeed(double value)
{
    // @TODO fix this: Pat's massive hack because he doesn't
    // have time to retro fit two systems with the ability
    // to know estimated speed up.
    if (value <= 16384) {
        this->realTimeSpeedUpFactor = value;
    }
}


void AdcsSimDataManager::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_connectionTimerId) {
        // Set up the response data to go out
        // Poll the connection event loop
        m_client.poll();
        // Check whether we are actually connected or still attempting
        if(m_client.isConnected()) {
            if(!m_isConnected) {
                m_isConnected = true;
                emit this->showMessage("Connection established!", 5000);
            }
        } else {
            if(m_isConnected) {
                m_isConnected = false;
                this->m_scSim.timeStamp = this->generateTimeStamp();
                this->receivedNewData = false;
            }
            // emit showMessage("Attempting to connect...");
        }

        long prevNumRun = this->m_scSim.timeStamp;
        this->m_scSim = m_client.getInboundData();
        if (this->m_scSim.timeStamp > prevNumRun && this->receivedNewData == false)
        {
            this->receivedNewData = true;
            this->realTimeSpeedUpFactor = this->m_scSim.realTimeSpeedUpFactor;
            emit simConnected();
            // Return here so slots and signals are processed
            // once before we reach updateSimObjects().
            return;
        }
        
        if (this->receivedNewData)
        {
            this->updateSimObjects();
            this->updateReturnData();
            this->m_client.setOutboundData(&this->m_scSimVisualization);
        }
    }
}

long AdcsSimDataManager::generateTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto nowInMilliSec = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = nowInMilliSec.time_since_epoch();
    
    return value.count();
}

double AdcsSimDataManager::getSimTime()
{
    return this->m_scSim.time;
}

void AdcsSimDataManager::updateSimObjects()
{
    SimObject       tempSimObject;
    QQuaternion     tempq;
    QVector3D       tempPosition;
    QVector4D       tempColor;
    classicElements oe;
    double          e[3];
    double          textureOffsetAngle   = 0.0;
    double          primaryBodyAngle     = 0.0;
    double          sunScaling           = 1.0;
    double          lightTime; // is the one-way light time between the observer and target in seconds
    double          celestialObject1State[6];
    double          celestialObject2State[6];
    double          sunState[6] = {0,0,0,0,0,0};
    int             i;
    double          helioRadius = 0.0;
    
    switch(m_scSim.celestialObject) {
        case CELESTIAL_SUN:
            spkez_c(399, m_scSim.ics.ET0 + m_scSim.time, "ECLIPJ2000", "NONE", 10, celestialObject1State, &lightTime);
            spkez_c(499, m_scSim.ics.ET0 + m_scSim.time, "ECLIPJ2000", "NONE", 10, celestialObject2State, &lightTime);
            spkez_c(10, m_scSim.ics.ET0 + m_scSim.time, "ECLIPJ2000", "NONE", 499, sunState, &lightTime);
            m_scSim.mu = MU_SUN;
            break;
        case CELESTIAL_MARS:
            spkez_c(401, m_scSim.ics.ET0 + m_scSim.time, "MARSIAU", "NONE", 499, celestialObject1State, &lightTime);
            spkez_c(402, m_scSim.ics.ET0 + m_scSim.time, "MARSIAU", "NONE", 499, celestialObject2State, &lightTime);
            spkez_c(10, m_scSim.ics.ET0 + m_scSim.time, "J2000", "NONE", 499, sunState, &lightTime);
            m_scSim.mu = MU_MARS;
            break;
        case CELESTIAL_EARTH:
            spkez_c(301, m_scSim.ics.ET0 + m_scSim.time, "ECLIPJ2000", "NONE", 399, celestialObject1State, &lightTime);
            spkez_c(10, m_scSim.ics.ET0 + m_scSim.time, "ECLIPJ2000", "NONE", 399, sunState, &lightTime);
            m_scSim.mu = MU_EARTH;
        default:
            break;
    }
    emit setOnOffDefaults(m_scSim.celestialObject);
    
    m_simObjects.clear();
    
    // The simulation unit length by default is km, the globalScaleFactor parameter
    // allows the scaling of everything to fit in clipping space
    // All top level objects of m_simObjects must have their scaleFactor parameter set
    // to this value and its position multiplied by this value
    float globalScaleFactor;
    switch (m_scSim.celestialObject) {
        case CELESTIAL_EARTH:
            globalScaleFactor = 1.0;
            break;
        case CELESTIAL_MARS:
            m_toggleableObjects.remove("Spacecraft Magnetic Field Vector");
            globalScaleFactor = 0.15f;
            break;
        case CELESTIAL_SUN:
            //m_toggleableObjects.remove("Spacecraft Magnetic Field Vector");
            sunScaling = 15.;
            globalScaleFactor = (float)(REQ_EARTH/AU);
            break;
        default:
            globalScaleFactor = 1.0;
            printf("Warning: received celestial object %d not implemented.\n", m_scSim.celestialObject);
            break;
    }
    
    float scScale;
    if(m_isSpacecraftTarget) {
        switch (m_scSim.celestialObject) {
            case CELESTIAL_EARTH:
                scScale = 3. * globalScaleFactor;
                break;
            case CELESTIAL_MARS:
                scScale = 3. * globalScaleFactor;
                break;
            case CELESTIAL_SUN:
                scScale = 0.5;
                break;
            default:
                scScale = 1.;
                break;
        }
    } else {
        switch (m_scSim.celestialObject) {
            case CELESTIAL_EARTH:
                scScale = 50. * globalScaleFactor;
                break;
            case CELESTIAL_MARS:
                scScale = 500 * globalScaleFactor;
                break;
            case CELESTIAL_SUN:
                scScale = 150.;
                break;
            default:
                scScale = 1.;
                break;
        }
    }
    
    // Draw planets first in case spacecraft has transparency
    QVector3D sunPosition;
    if (v3Norm(sunState)>0.5) {
        sunPosition = QVector3D(sunState[0], sunState[1], sunState[2]);
    } else {
        sunPosition = QVector3D(1,0,0);
    }
    helioRadius = sunPosition.length();
    m_scSim.sHatN[0] = sunPosition[0]/helioRadius;
    m_scSim.sHatN[1] = sunPosition[1]/helioRadius;
    m_scSim.sHatN[2] = sunPosition[2]/helioRadius;
    double dcm_BN[3][3];
    double dcm_NB[3][3];
    
    MRP2C(m_scSim.sigma, dcm_NB);
    m33Transpose(dcm_NB, dcm_BN);
    m33MultV3(dcm_NB, m_scSim.sHatN, m_scSim.sHatB);
    
    // Draw planets first in case spacecraft has transparency
    SimObject primaryBody;
    primaryBody.position = QVector3D(0, 0, 0);
    if(m_scSim.celestialObject == CELESTIAL_EARTH) {
        primaryBody.name         = "Earth";
        primaryBody.geometryName = "Earth";
        primaryBody.scaleFactor  = globalScaleFactor;
        textureOffsetAngle       = - 180.0*D2R;
        primaryBodyAngle         = m_scSim.gamma + textureOffsetAngle;
        m_simObjects.push_back(createSun(sunPosition, globalScaleFactor));
    } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
        primaryBody.name         = "Mars";
        primaryBody.geometryName = "Mars";
        primaryBody.scaleFactor  = globalScaleFactor;
        textureOffsetAngle       = - 90.0*D2R;
        primaryBodyAngle         = m_scSim.gamma + textureOffsetAngle;
        m_simObjects.push_back(createSun(sunPosition, globalScaleFactor));
    } else if(m_scSim.celestialObject == CELESTIAL_SUN) {
        primaryBody.name         = "Sun";
        primaryBody.geometryName = "Sun";
        primaryBody.scaleFactor  = sunScaling * globalScaleFactor;
        textureOffsetAngle       = 0.;
        primaryBodyAngle         = 0.;
    }
    
    // Resolve orientation of primary body
    
    primaryBody.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(primaryBodyAngle/2.));
    
    if(m_toggleableObjects["Planet-Centered Fixed Axes"]) {
        SimObject temp;
        temp = createXAxis();
        temp.quaternion = QQuaternion(cos(textureOffsetAngle/2.), 0.0, 0.0, sin(-textureOffsetAngle/2.))*temp.quaternion;
        primaryBody.simObjects.push_back(temp);
        temp = createYAxis();
        temp.quaternion = QQuaternion(cos(textureOffsetAngle/2.), 0.0, 0.0, sin(-textureOffsetAngle/2.))*temp.quaternion;
        primaryBody.simObjects.push_back(temp);
        temp = createZAxis();
        temp.quaternion = QQuaternion(cos(textureOffsetAngle/2.), 0.0, 0.0, sin(-textureOffsetAngle/2.))*temp.quaternion;
        primaryBody.simObjects.push_back(temp);
    }
    
    if(m_toggleableObjects["Planet-Centered Inertial Axes"]) {
        SimObject temp;
        double axisScale = 1.25;
        if (m_scSim.celestialObject == CELESTIAL_SUN) {
            axisScale = 2.0;
        }
        temp = createXAxis();
        temp.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.))*temp.quaternion;
        temp.scaleFactor = axisScale;
        primaryBody.simObjects.push_back(temp);
        temp = createYAxis();
        temp.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.))*temp.quaternion;
        temp.scaleFactor = axisScale;
        primaryBody.simObjects.push_back(temp);
        temp = createZAxis();
        temp.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.))*temp.quaternion;
        temp.scaleFactor = axisScale;
        primaryBody.simObjects.push_back(temp);
    }
    
    if(m_toggleableObjects["Spacecraft Orbit"]) {
        classicElements oe;
        rv2elem(m_scSim.mu, m_scSim.r_N, m_scSim.v_N, &oe);
        tempSimObject = createOrbit(oe, 0, m_scSim.mu, "Spacecraft");
        // Undo the Earth's rotation before placing orbit on screen
        tempSimObject.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.));
        tempSimObject.scaleFactor = 1.0/sunScaling;
        primaryBody.simObjects.push_back(tempSimObject);
        if (oe.alpha<0.) {
            tempSimObject = createOrbit(oe, 1, m_scSim.mu,"SpacecraftHyperbolicDeparture");
            tempSimObject.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.));
            tempSimObject.scaleFactor = 1.0/sunScaling;
            primaryBody.simObjects.push_back(tempSimObject);
        }
    }
    
    // Place Earth's moon
    if (m_scSim.celestialObject == CELESTIAL_EARTH) {
        SimObject moon1;
        moon1.name = "Moon";
        moon1.geometryName = "Moon";
        moon1.position = QVector3D(celestialObject1State[0], celestialObject1State[1], celestialObject1State[2]) * globalScaleFactor;
        moon1.scaleFactor = 1.0f * globalScaleFactor;
        m_simObjects.push_back(moon1);
        
        // Create moon's orbit
        if(m_toggleableObjects["Celestial Object Orbits"]) {
            double tempR[3] = {celestialObject1State[0], celestialObject1State[1], celestialObject1State[2]};
            double tempV[3] = {celestialObject1State[3], celestialObject1State[4], celestialObject1State[5]};
            rv2elem(MU_EARTH, tempR, tempV, &oe);
            tempSimObject = createOrbit(oe, 0, MU_EARTH,"Moon");
            // Undo the planets's rotation before placing orbit on screen
            tempSimObject.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.));
            primaryBody.simObjects.push_back(tempSimObject);
        }
    }
    
    // Place Mars moons
    if (m_scSim.celestialObject == CELESTIAL_MARS) {
        SimObject moon1;
        moon1.name = "Phobos";
        moon1.geometryName = "Phobos";
        moon1.position = QVector3D(celestialObject1State[0], celestialObject1State[1], celestialObject1State[2]) * globalScaleFactor;
        moon1.scaleFactor = 1.0f * globalScaleFactor; //30.0f
        m_simObjects.push_back(moon1);
        
        SimObject moon2;
        moon2.name = "Deimos";
        moon2.geometryName = "Deimos";
        moon2.position = QVector3D(celestialObject2State[0], celestialObject2State[1], celestialObject2State[2]) * globalScaleFactor;
        moon2.scaleFactor = 1.0f * globalScaleFactor; //20.0f
        m_simObjects.push_back(moon2);
        
        if(m_toggleableObjects["Celestial Object Orbits"]) {
            // Create Phobos orbit
            double tempR[3] = {celestialObject1State[0], celestialObject1State[1], celestialObject1State[2]};
            double tempV[3] = {celestialObject1State[3], celestialObject1State[4], celestialObject1State[5]};
            rv2elem(MU_MARS, tempR, tempV, &oe);
            tempSimObject = createOrbit(oe, 0, MU_MARS,"Phobos");
            // Undo the planets's rotation before placing orbit on screen
            tempSimObject.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.));
            primaryBody.simObjects.push_back(tempSimObject);
            
            // Create Deimos orbit
            v3Set(celestialObject2State[0], celestialObject2State[1], celestialObject2State[2], tempR);
            v3Set(celestialObject2State[3], celestialObject2State[4], celestialObject2State[5], tempV);
            rv2elem(MU_MARS, tempR, tempV, &oe);
            tempSimObject = createOrbit(oe, 0, MU_MARS,"Deimos");
            // Undo the planets's rotation before placing orbit on screen
            tempSimObject.quaternion = QQuaternion(cos(primaryBodyAngle/2.), 0.0, 0.0, sin(-primaryBodyAngle/2.));
            primaryBody.simObjects.push_back(tempSimObject);
        }
    }
    
    if (m_scSim.celestialObject == CELESTIAL_SUN) {
        SimObject temp;
        double    planetScaling = 1200.;
        temp.name = "Earth";
        temp.geometryName = "Earth";
        temp.position = QVector3D(celestialObject1State[0], celestialObject1State[1], celestialObject1State[2]) * globalScaleFactor;
        temp.scaleFactor = planetScaling * globalScaleFactor;
        m_simObjects.push_back(temp);
        
        temp.name = "Mars";
        temp.geometryName = "Mars";
        temp.position = QVector3D(celestialObject2State[0], celestialObject2State[1], celestialObject2State[2]) * globalScaleFactor;
        temp.scaleFactor = planetScaling * REQ_MARS/REQ_EARTH * globalScaleFactor;
        m_simObjects.push_back(temp);
        
        if(m_toggleableObjects["Celestial Object Orbits"]) {
            // Create Earth orbit
            double tempR[3] = {celestialObject1State[0], celestialObject1State[1], celestialObject1State[2]};
            double tempV[3] = {celestialObject1State[3], celestialObject1State[4], celestialObject1State[5]};
            rv2elem(MU_SUN, tempR, tempV, &oe);
            tempSimObject = createOrbit(oe, 0, MU_SUN,"Earth");
            tempSimObject.scaleFactor = 1./sunScaling;
            primaryBody.simObjects.push_back(tempSimObject);
            
            // Create Mars orbit
            v3Set(celestialObject2State[0], celestialObject2State[1], celestialObject2State[2], tempR);
            v3Set(celestialObject2State[3], celestialObject2State[4], celestialObject2State[5], tempV);
            rv2elem(MU_SUN, tempR, tempV, &oe);
            tempSimObject = createOrbit(oe, 0, MU_SUN,"Mars");
            tempSimObject.scaleFactor = 1./sunScaling;
            primaryBody.simObjects.push_back(tempSimObject);
        }
    }
    
    m_simObjects.push_back(primaryBody);
    
    SimObject spacecraft;
    spacecraft.name = "Spacecraft";
    spacecraft.position = QVector3D(m_scSim.r_N[0], m_scSim.r_N[1], m_scSim.r_N[2]) * globalScaleFactor;
    double spacecraftq[4];
    MRP2EP(m_scSim.sigma, spacecraftq);
    spacecraft.quaternion = QQuaternion(spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3]);
    spacecraft.scaleFactor = scScale;
    spacecraft.geometryName = "Spacecraft";
    if(m_toggleableObjects["Spacecraft Body Frame Axes"]) {
        SimObject temp;
        temp = createXAxis();
        spacecraft.simObjects.push_back(temp);
        temp = createYAxis();
        spacecraft.simObjects.push_back(temp);
        temp = createZAxis();
        spacecraft.simObjects.push_back(temp);
    }
    if(m_toggleableObjects["Spacecraft Hill Frame Axes"]) {
        SimObject temp;
        classicElements oe;
        rv2elem(m_scSim.mu, m_scSim.r_N, m_scSim.v_N, &oe);
        e[0] = oe.Omega;
        e[1] = oe.i;
        e[2] = oe.omega + oe.f;
        double q[4];
        Euler3132EP(e, q);
        temp = createXAxis(QVector4D(0.98f, 0.6f, 0.6f, 1.0f));
        temp.quaternion = QQuaternion(-spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3])
        * QQuaternion(q[0], q[1], q[2], q[3])
        * temp.quaternion;
        spacecraft.simObjects.push_back(temp);
        temp = createYAxis(QVector4D(0.69f, 0.87f, 0.54f, 1.0f));
        temp.quaternion = QQuaternion(-spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3])
        * QQuaternion(q[0], q[1], q[2], q[3])
        * temp.quaternion;
        spacecraft.simObjects.push_back(temp);
        temp = createZAxis(QVector4D(0.65f, 0.81f, 0.89f, 1.0f));
        temp.quaternion = QQuaternion(-spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3])
        * QQuaternion(q[0], q[1], q[2], q[3])
        * temp.quaternion;
        spacecraft.simObjects.push_back(temp);
    }
    if(m_toggleableObjects["Spacecraft Velocity Frame Axes"]) {
        SimObject temp;
        classicElements oe;
        rv2elem(m_scSim.mu, m_scSim.r_N, m_scSim.v_N, &oe);
        e[0] = oe.Omega;
        e[1] = oe.i;
        e[2] = oe.omega + oe.f - atan(oe.e*sin(oe.f)/(1.+oe.e*cos(oe.f)));
        double q[4];
        Euler3132EP(e, q);
        temp = createXAxis(QVector4D(0.98f, 0.6f, 0.6f, 1.0f));
        temp.quaternion = QQuaternion(-spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3])
        * QQuaternion(q[0], q[1], q[2], q[3])
        * temp.quaternion;
        spacecraft.simObjects.push_back(temp);
        temp = createYAxis(QVector4D(0.69f, 0.87f, 0.54f, 1.0f));
        temp.quaternion = QQuaternion(-spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3])
        * QQuaternion(q[0], q[1], q[2], q[3])
        * temp.quaternion;
        spacecraft.simObjects.push_back(temp);
        temp = createZAxis(QVector4D(0.65f, 0.81f, 0.89f, 1.0f));
        temp.quaternion = QQuaternion(-spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3])
        * QQuaternion(q[0], q[1], q[2], q[3])
        * temp.quaternion;
        spacecraft.simObjects.push_back(temp);
    }
    if(m_toggleableObjects["Spacecraft Sun-Direction Vector"]) {
        QVector4D color(1.0f, 1.0f, 0.0f, 1.0f);
        SimObject temp;
        temp.name = "Sun-Direction";
        temp.position = QVector3D(0, 0, 0);
        temp.quaternion = SimObject::computeRotation(QVector3D(m_scSim.sHatB[0], m_scSim.sHatB[1], m_scSim.sHatB[2]));
        temp.geometryName = "UnitLine";
        temp.scaleFactor = 1.1f;
        temp.scaleByParentBoundingRadii = true;
        temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
        spacecraft.simObjects.push_back(temp);
    }
    if(m_toggleableObjects["Spacecraft Earth-Direction Vector"]) {
        QVector4D color(0.0f, 1.0f, 1.0f, 1.0f);
        SimObject temp;
        temp.name = "Earth-Direction";
        temp.position = QVector3D(0, 0, 0);
        temp.quaternion = SimObject::computeRotation(QVector3D(m_scSim.earthHeadingSim_B[0],m_scSim.earthHeadingSim_B[1],m_scSim.earthHeadingSim_B[2]));
        temp.geometryName = "UnitLine";
        temp.scaleFactor = 1.1f;
        temp.scaleByParentBoundingRadii = true;
        temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
        spacecraft.simObjects.push_back(temp);
    }
    if(m_toggleableObjects["Spacecraft Magnetic Field Vector"]) {
        //QVector4D color(0.6f, 0.3f, 0.6f, 1.0f);
        QVector4D color(220 / 255.0f, 50 / 255.0f, 220 / 255.0f, 1.0f);
        SimObject temp;
        temp.name = "Magnetic Field";
        temp.position = QVector3D(0, 0, 0);
        temp.quaternion = QQuaternion(-spacecraftq[0], spacecraftq[1], spacecraftq[2], spacecraftq[3])
        * SimObject::computeRotation(QVector3D(m_scSim.B_N[0], m_scSim.B_N[1], m_scSim.B_N[2]));
        temp.geometryName = "UnitLine";
        temp.scaleFactor = 1.1f;
        temp.scaleByParentBoundingRadii = true;
        temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(color));
        spacecraft.simObjects.push_back(temp);
    }
    
    // Spacecraft ACS Thrusters
    std::vector<Thruster>::iterator itAcsThrst;
    for (itAcsThrst = m_scSim.acsThrusters.begin(); itAcsThrst != m_scSim.acsThrusters.end(); itAcsThrst++)
    {
        if((*itAcsThrst).level > 0)
        {
            tempq = SimObject::computeRotation(QVector3D((*itAcsThrst).gt_B[0], (*itAcsThrst).gt_B[1], (*itAcsThrst).gt_B[2]));
            tempPosition = QVector3D((*itAcsThrst).r_B[0], (*itAcsThrst).r_B[1], (*itAcsThrst).r_B[2]) * scScale;
            tempColor = QVector4D(1.0f, 0.5f, 0.0f, 1.0f);
            double thrusterScaleFactor = 1.0f;
            if (m_isSpacecraftTarget) {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    thrusterScaleFactor = 0.25f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    thrusterScaleFactor = 1.0f;
                } else if(m_scSim.celestialObject == CELESTIAL_SUN) {
                    thrusterScaleFactor = 1.5f;
                } else {
                    thrusterScaleFactor = 1.0f;
                }
            } else {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    thrusterScaleFactor = 0.25f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    thrusterScaleFactor = 0.1f;
                } else {
                    thrusterScaleFactor = 0.1f;
                }
            }
            SimObject thruster = createThruster(tempPosition,
                                                tempq, 100., (*itAcsThrst).level/100, thrusterScaleFactor, tempColor);
            spacecraft.simObjects.push_back(thruster);
        }
    }
    
    // Spacecraft DV Thrusters
    std::vector<Thruster>::iterator dvIt;
    for (dvIt = m_scSim.dvThrusters.begin(); dvIt != m_scSim.dvThrusters.end(); ++dvIt)
    {
        if((*dvIt).level > 0)
        {
            tempq = SimObject::computeRotation(QVector3D(-(*dvIt).gt_B[0], -(*dvIt).gt_B[1], -(*dvIt).gt_B[2]));
            tempPosition = QVector3D((*dvIt).r_B[0], (*dvIt).r_B[1], (*dvIt).r_B[2]) * scScale*0.7;
            double thrusterScaleFactor = 1.0f;
            double plumeScale = 2.0;
            if (m_isSpacecraftTarget) {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    thrusterScaleFactor = 0.25f*plumeScale;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    thrusterScaleFactor = 1.0f*plumeScale;
                } else {
                    thrusterScaleFactor = 1.0f*plumeScale;
                }
            } else {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    thrusterScaleFactor = 0.25f*plumeScale;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    thrusterScaleFactor = 0.1f*plumeScale;
                } else {
                    thrusterScaleFactor = 0.1f*plumeScale;
                }
            }
            SimObject thruster = createThruster(tempPosition, tempq, 25.0f*plumeScale, (*dvIt).level/50, thrusterScaleFactor);
            spacecraft.simObjects.push_back(thruster);
        }
    }
    
    // Spacecraft CSS
    QVector4D colorSolid;
    QVector4D colorOpaque;
    for(i = 0; i < NUM_CSS; i ++) {
        tempq = SimObject::computeRotation(QVector3D(m_scSim.css[i].nHatB[0], m_scSim.css[i].nHatB[1], m_scSim.css[i].nHatB[2]));
        tempPosition = QVector3D(m_scSim.css[i].r_B[0],m_scSim.css[i].r_B[1],m_scSim.css[i].r_B[2]) * scScale;
        
        if(m_scSim.css[i].state!=COMPONENT_FAULT){
            
            if (m_scSim.css[i].directValue > 0) {
                colorSolid = QVector4D(1.0f, 1.0f, 0.0f, 1.0f);
                colorOpaque = QVector4D(1.0f, 1.0f, 0.0f, 0.4f);
            } else {
                colorSolid = QVector4D(0.6f, 0.3f, 0.6f, 1.0f);
                colorOpaque = QVector4D(0.6f, 0.3f, 0.6f, 0.4f);
            }
        }
        else
        {
            colorSolid = QVector4D(1.0f, 0.0f, 0.0f, 1.0f);
            colorOpaque = QVector4D(1.0f, 0.0f, 0.0f, 0.4f);
        }
        if (m_toggleableObjects["CSS Field Of View"]) {
            tempSimObject = createFieldOfView(tempPosition, tempq, 1.0f, 2*m_scSim.css[i].fov*180/M_PI
                                              , colorOpaque);
            double fieldOfViewScaleFactor = 1.0f;
            if (m_isSpacecraftTarget) {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    fieldOfViewScaleFactor = 0.35f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    fieldOfViewScaleFactor = 1.0f;
                } else {
                    fieldOfViewScaleFactor = 1.0f;
                }
            } else {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    fieldOfViewScaleFactor = 0.075f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    fieldOfViewScaleFactor = 0.05f;
                } else {
                    fieldOfViewScaleFactor = 0.06f;
                }
            }
            tempSimObject.scaleFactor = fieldOfViewScaleFactor;
            spacecraft.simObjects.push_back(tempSimObject);
        }
        if (m_toggleableObjects["CSS Photo Diode Normals"]) {
            tempSimObject = createFadingAxis(tempPosition, tempq, colorSolid);
            tempSimObject.name = "cssSensorNormal";
            spacecraft.simObjects.push_back(tempSimObject);
        }
    }
    
    // Spacecraft RW
    Legend m_legendColors;
    if (m_toggleableObjects["Reaction Wheel Pyramid"])
    {
        QVector4D colorRW = QVector4D(1.0f, 1.0f, 1.0f, 0.25f);
        std::vector<RWSim>::iterator itRw;
        for (itRw = m_scSim.reactionWheels.begin(); itRw != m_scSim.reactionWheels.end(); itRw++)
        {
            // Set color
            if (itRw->state == COMPONENT_ON)
            {
                colorRW = QVector4D(180 / 255.0f, 160 / 255.0f, 240 / 255.0f, 1.0f);
                float percent =  fabs(itRw->Omega / itRw->Omega_max * 100);
                int range = 100 / (m_legendColors.colorPalette.size()-1);
                int colorIndex = 0;
                for (int i = 0; i < m_legendColors.colorPalette.size(); i++)
                {
                    if (percent <= range * (i + 1))
                    {
                        colorIndex = i;
                        break;
                    }
                }
                colorRW = QVector4D(m_legendColors.colorPalette[colorIndex].red() / 255.0,
                                    m_legendColors.colorPalette[colorIndex].green() / 255.0,
                                    m_legendColors.colorPalette[colorIndex].blue() / 255.0,
                                    1.0f);
            } else if (itRw->state == COMPONENT_OFF && itRw->resetCounter>=3) {
                colorRW = QVector4D(0.7f, 0.0f, 0.0f, 1.0f);
            } else {
                colorRW = QVector4D(47.0 / 255, 78.0 / 255, 78.0 / 255, 1.0f);
            }
            
            
            colorRW = QVector4D(47.0 / 255, 78.0 / 255, 78.0 / 255, 1.0f);
            float percent_w =  fabs(itRw->Omega / itRw->Omega_max * 100);
            
            if (percent_w < 10)
            {
                colorRW = QVector4D(100 / 255, 200 / 255, 78.0 / 255, 1.0f);
            }
            else if (percent_w < 20)
            {
                colorRW = QVector4D(0.0f, 1.0f, 0.0f, 1.0f);
            }
            else if (percent_w < 30)
            {
                colorRW = QVector4D(0.0f, 0.0f, 1.0f, 1.0f);
            }
            else if (percent_w < 30)
            {
                colorRW = QVector4D(0.5f, 0.0f, 0.5f, 1.0f);
            }
            else { colorRW = QVector4D(1.f, 1.f, 1.f, 1.0f); }
            //
            //            QColor(qRgb(110, 130, 255)));
            //            colorPalette.append (QColor(qRgb(100, 200, 255)));
            //            colorPalette.append (QColor(qRgb(110, 255, 130)));
            //            colorPalette.append (QColor(qRgb(255, 255, 130)));
            //            colorPalette.append (QColor(qRgb(255, 170,100)));
            //            colorPalette.append (QColor(qRgb(255, 0, 70)));
            
            
            // RW Spin Normals
            SimObject temp;
            temp.name = "RW-Axis";
            temp.position = QVector3D(itRw->rWB_S[0],itRw->rWB_S[1],itRw->rWB_S[2]) * scScale;
            temp.quaternion = SimObject::computeRotation(QVector3D(itRw->gsHat_S[0], itRw->gsHat_S[1], itRw->gsHat_S[2]));
            temp.geometryName = "UnitLine";
            temp.scaleFactor = 1.25f;
            temp.scaleByParentBoundingRadii = true;
            temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(colorRW));
            spacecraft.simObjects.push_back(temp);
            
            // RW Disks
            tempPosition = QVector3D(itRw->rWB_S[0], itRw->rWB_S[1], itRw->rWB_S[2]) * scScale * 6;
            tempq = SimObject::computeRotation(QVector3D(itRw->gsHat_S[0], itRw->gsHat_S[1], itRw->gsHat_S[2]));
            
            if (itRw->Omega < 0)
            {
                tempSimObject = createDiskRW(tempPosition, tempq, 1.0f, colorRW, false);
            } else {
                tempSimObject = createDiskRW(tempPosition, tempq, 1.0f, colorRW, true);
            }
            
            double rwDiskScaleFactor = 1.0f;
            if (m_isSpacecraftTarget) {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    rwDiskScaleFactor = 0.35f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    rwDiskScaleFactor = 1.0f;
                } else {
                    rwDiskScaleFactor = 1.0f;
                }
            } else {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    rwDiskScaleFactor = 0.075f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    rwDiskScaleFactor = 0.05f;
                } else {
                    rwDiskScaleFactor = 0.06f;
                }
            }
            tempSimObject.scaleFactor = rwDiskScaleFactor;
            spacecraft.simObjects.push_back(tempSimObject);
        }
        emit setOnOffLegendRW(true);
    } else {
        emit setOnOffLegendRW(false);
    }
    
    // Torque Rods
    if (m_toggleableObjects["Torque Rod Pyramid"])
    {
        QVector4D colorTR = QVector4D(1.0f, 1.0f, 1.0f, 0.25f);
        for(i = 0; i < NUM_TR; i ++)
        {
            // Set color
            if (m_scSim.tr[i].state == COMPONENT_ON)
            {
                colorTR = QVector4D(1.0, 150 / 255.0, 1.0, 1.0f);
                float percent =  fabs(m_scSim.tr[i].u / 6.0 * 100);
                int range = 100 / (m_legendColors.colorPalette.size()-1);
                int colorIndex = 0;
                for (int i = 0; i < m_legendColors.colorPalette.size(); i++)
                {
                    if (percent <= range * (i + 1))
                    {
                        colorIndex = i;
                        break;
                    }
                }
                colorTR = QVector4D(m_legendColors.colorPalette[colorIndex].red() / 255.0,
                                    m_legendColors.colorPalette[colorIndex].green() / 255.0,
                                    m_legendColors.colorPalette[colorIndex].blue() / 255.0,
                                    1.0f);
            } else {
                colorTR = QVector4D(47.0 / 255, 78.0 / 255, 78.0 / 255, 1.0f);
            }
            
            // TR Dipole Axes
            SimObject temp;
            temp.name = "TR-Axis";
            temp.position = QVector3D(m_scSim.tr[i].r_B[0], m_scSim.tr[i].r_B[1], m_scSim.tr[i].r_B[2]) * scScale;
            temp.quaternion = SimObject::computeRotation(QVector3D(m_scSim.tr[i].dipoleAxis[0], m_scSim.tr[i].dipoleAxis[1], m_scSim.tr[i].dipoleAxis[2]));
            temp.geometryName = "UnitLine";
            temp.scaleFactor = 1.75f;
            temp.scaleByParentBoundingRadii = true;
            temp.defaultMaterialOverride = QSharedPointer<Geometry::MaterialInfo>(new Geometry::MaterialInfo(colorTR));
            spacecraft.simObjects.push_back(temp);
            
            // TR Bars
            tempPosition = QVector3D(m_scSim.tr[i].r_B[0], m_scSim.tr[i].r_B[1], m_scSim.tr[i].r_B[2]) * scScale;
            tempq = SimObject::computeRotation(QVector3D(m_scSim.tr[i].dipoleAxis[0], m_scSim.tr[i].dipoleAxis[1], m_scSim.tr[i].dipoleAxis[2]));
            tempSimObject = createTorqueBar(tempPosition, tempq, 1.0f, colorTR);
            
            double torqueBarScaleFactor = 1.0f;
            if (m_isSpacecraftTarget) {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    torqueBarScaleFactor = 0.35f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    torqueBarScaleFactor = 1.0f;
                } else {
                    torqueBarScaleFactor = 1.0f;
                }
            } else {
                if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                    torqueBarScaleFactor = 0.075f;
                } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                    torqueBarScaleFactor = 0.05f;
                } else {
                    torqueBarScaleFactor = 0.06f;
                }
            }
            tempSimObject.scaleFactor = torqueBarScaleFactor;
            spacecraft.simObjects.push_back(tempSimObject);
        }
        emit setOnOffLegendTR(true);
    } else {
        emit setOnOffLegendTR(false);
    }
    
    // Spacecraft ST
    
    QVector4D colorST = QVector4D(0.0f, 1.0f, 1.0f, 1.0f);
    if (m_toggleableObjects["Star Tracker Pointing Normals"])
    {
        tempq = SimObject::computeRotation(QVector3D(m_scSim.st.gs_head1[0],m_scSim.st.gs_head1[1],m_scSim.st.gs_head1[2]) * scScale);
        tempPosition = QVector3D(m_scSim.st.r_B_head1[0],m_scSim.st.r_B_head1[1],m_scSim.st.r_B_head1[2])* scScale;
        SimObject starTrackerPointer1 = createFadingAxis(tempPosition, tempq, colorST);
        spacecraft.simObjects.push_back(starTrackerPointer1);
        
        tempq = SimObject::computeRotation(QVector3D(m_scSim.st.gs_head2[0],m_scSim.st.gs_head2[1],m_scSim.st.gs_head2[2]) * scScale );
        tempPosition = QVector3D(m_scSim.st.r_B_head2[0],m_scSim.st.r_B_head2[1],m_scSim.st.r_B_head2[2])* scScale;
        SimObject starTrackerPointer2 = createFadingAxis(tempPosition, tempq, colorST);
        spacecraft.simObjects.push_back(starTrackerPointer2);
    }
    
    if (m_toggleableObjects["Star Tracker Field of View"])
    {
        // Scale Factor
        double stFovScaleFactor = 1.0f;
        if (m_isSpacecraftTarget) {
            if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                stFovScaleFactor = 0.35f;
            } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                stFovScaleFactor = 1.0f;
            } else {
                stFovScaleFactor = 1.0f;
            }
        } else {
            if(m_scSim.celestialObject == CELESTIAL_EARTH) {
                stFovScaleFactor = 0.075f;
            } else if(m_scSim.celestialObject == CELESTIAL_MARS) {
                stFovScaleFactor = 0.05f;
            } else {
                stFovScaleFactor = 0.06f;
            }
        }
        // FOV (cuboid) head 1
        tempq = SimObject::computeRotation(QVector3D(m_scSim.st.gs_head1[0], m_scSim.st.gs_head1[1], m_scSim.st.gs_head1[2]));
        tempPosition = QVector3D(m_scSim.st.r_B_head1[0], m_scSim.st.r_B_head1[1], m_scSim.st.r_B_head1[2]) * scScale;
        SimObject starTrackerFOV1 = createStarTrackerFOV(tempPosition, tempq, 1.0f, M_PI/8, colorST);
        starTrackerFOV1.scaleFactor = stFovScaleFactor;
        spacecraft.simObjects.push_back(starTrackerFOV1);
        
        // FOV (cuboid) head 2
        tempq = SimObject::computeRotation(QVector3D(m_scSim.st.gs_head2[0], m_scSim.st.gs_head2[1], m_scSim.st.gs_head2[2]));
        tempPosition = QVector3D(m_scSim.st.r_B_head2[0], m_scSim.st.r_B_head2[1], m_scSim.st.r_B_head2[2]) * scScale;
        SimObject starTrackerFOV2 = createStarTrackerFOV(tempPosition, tempq, 1.0f, M_PI/8, colorST);
        starTrackerFOV2.scaleFactor = stFovScaleFactor;
        spacecraft.simObjects.push_back(starTrackerFOV2);
        
    }
    
    m_simObjects.push_back(spacecraft);
    
    m_preferredCameraTarget = m_simObjects.length() - 1;
    
    emit simDataUpdated();
}

void AdcsSimDataManager::updateReturnData()
{
    this->m_scSimVisualization.realTimeSpeedUpFactor = this->realTimeSpeedUpFactor;
}
