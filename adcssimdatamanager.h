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
#ifndef ADCSSIMDATAMANAGER_H
#define ADCSSIMDATAMANAGER_H

#include "simdatamanager.h"
#include "TcpSerializeClient.hpp"
#include "SpacecraftSimDefinitions.h"

#include <QTimerEvent>
#include <boost/asio.hpp>

extern "C" {
#include "utilities/linearAlgebra.h"
}

class AdcsSimDataManager : public SimDataManager
{
    Q_OBJECT
public:
    explicit AdcsSimDataManager(QObject *parent = 0);
    ~AdcsSimDataManager();

    // Methods for loading in data
    virtual bool openConnection(QString ipAddress, QString port);
    virtual bool closeConnection();
    virtual bool openFile(QString filename);
    virtual bool closeFile();

    // Methods for dispersing data loaded in
    virtual QVector3D getLightPosition();

    SpacecraftSim *getSpacecraftSim() {
        return &m_scSim;
    }
//    SpacecraftSimVisualization *getSpacecraftSimVisualization()
//    {
//        return &m_scSimVisualization;
//    }
    
signals:
    void setOnOffLegendRW(bool);
    void setOnOffLegendTR(bool);
    void setOnOffDefaults(CelestialObject_t);
    void simConnected();

public slots:
    // Set the simulation time
    virtual void setSimTime(double time);
    double getSimTime();
//    void perturbRates(void);
//    void setControlState(CommandedState_t);
//    void setAdcsState(ADCSState_t);
//    void setThrustingControlMode(int);
    void setPlaySpeed(double value);
//    void setGravityGradientTorque(int);
//    void setReactionWheelJitter(int);
//    void setGravityPerturbModel(int);
//    void setAtmosDragModel(int);
//    void setSrpModel(int);
//    
//    void setOnOffState(ComponentState_t, int);
//    void setStateCSS(ComponentState_t, int);
//    void setStateTR(ComponentState_t, int);
//    void setStatePOD(ComponentState_t);
//    void setStateIRU(ComponentState_t);
//    void setStateTAM(ComponentState_t);
//    
//    void setAlbedoModel(AlbedoModel_t);
//    void setEarthMagField(EarthMagFieldModel_t);
//    void setNewManeuver(double, double);

protected:
    virtual void timerEvent(QTimerEvent *event);

private:
    void updateSimObjects();
    long generateTimeStamp();
    void updateReturnData();

private:
    TcpSerializeClient<SpacecraftSim, SpacecraftSim> m_client;
    bool m_isConnected;
    int m_connectionTimerId;
    SpacecraftSim m_scSim;
    SpacecraftSim m_scSimVisualization;
    double   realTimeSpeedUpFactor;
    bool receivedNewData;
};

#endif // ADCSSIMDATAMANAGER_H
