#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <zmq.hpp>
#include <QString>
#include <QStringList>
#include <string>
#include <iostream>
#include <unistd.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QByteArray>
#include <Tree.h>
#include <Hyperrectangle.h>
#include <config.h>

class Connector
{
private:
    zmq::context_t *ctx;
    zmq::socket_t * socket;
    Tree<int> * tree;

    QString base64_encode(QString string){
        QByteArray ba;
        ba.append(string);
        return ba.toBase64();
    }

    QString base64_decode(QString string){
        QByteArray ba;
        ba.append(string);
        return QByteArray::fromBase64(ba);
    }

    QString makeFunction(QJsonObject & data)
    {
        int func = (int) data["function"].toDouble();
        switch (func) {
        case 1:
            return insert(data);
            break;
        default:
            return select(data);
            break;
        }

        return QString("");
    }

    QString select(QJsonObject & data)
    {
        std::cout << "Select" << std::endl;

        points geometry = this->parsePoints(data);
        int value = (int) data["value"].toString().toDouble();

        Hyperrectangle boundingbox(geometry);
        vector<int> objects;
        this->tree->Search(boundingbox, objects);
        QJsonObject o;
        o["status"]=QString("ok");
        QJsonArray points;
        for(int object : objects)
        {
            points.push_back(QString::number(object));
        }
        o["objects"]=points;
        QJsonDocument ans(o);
        return ans.toJson().toBase64();
    }

    QString insert(QJsonObject & data)
    {
        std::cout << "Insert " << data["value"].toString().toStdString() << std::endl;

        points geometry = this->parsePoints(data);

        int value = (int) data["value"].toString().toDouble();

        Hyperrectangle boundingbox(geometry);
        this->tree->insert(boundingbox, value);

        QJsonObject o;
        o["status"]=QString("ok");
        QJsonDocument ans(o);
        return ans.toJson().toBase64();
    }

    points parsePoints(QJsonObject & data)
    {
        points ans;
        QJsonArray geometry = data["geometry"].toArray();

        for(const QJsonValue & tmpDimension : geometry)
        {
            QJsonArray dimension = tmpDimension.toArray();
            ans.push_back(make_pair(dimension.first().toString("").toDouble(),
                                    dimension.last().toString("").toDouble()));
        }
        return ans;
    }

public:
    Connector()
    {
        this->ctx = new zmq::context_t(1);
        this->socket = new zmq::socket_t(*this->ctx, ZMQ_REP);
        this->tree = new Tree<int>(2);
    }

    void listen()
    {
        this->socket->bind("tcp://*:2116");
        zmq::message_t msg;
        std::cout << "Starting..." << std::endl;
        while(this->socket->recv(&msg))
        {
            std::cout << "recibido" << std::endl;

            char* msg_data = new char[msg.size()];
            memcpy(msg_data, msg.data(), msg.size());
            QString query(msg_data);

            std::cout << this->base64_decode(query).toStdString() << std::endl;
            QJsonDocument json = QJsonDocument::fromJson(this->base64_decode(query).toUtf8());
            QJsonObject dataObject = json.object();

            QString ans = this->makeFunction(dataObject);

            zmq::message_t reply (ans.size());
            memcpy ((void *) reply.data (), ans.toStdString().c_str(), ans.size());
            this->socket->send(reply);
        }
    }
};

#endif // CONNECTOR_H
