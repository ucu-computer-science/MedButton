///* Source https://stackoverflow.com/questions/49344638/adding-markers-places-to-qt-qml-maps */

//#ifndef MARKERMODEL_H
//#define MARKERMODEL_H

//#include <QAbstractListModel>
//#include <QGeoCoordinate>

//class MarkerModel : public QAbstractListModel
//{
//    Q_OBJECT

//public:
//    using QAbstractListModel::QAbstractListModel;
//    enum MarkerRoles{positionRole = Qt::UserRole + 1};

//    Q_INVOKABLE void addMarker(const QGeoCoordinate &coordinate){
//        beginInsertRows(QModelIndex(), rowCount(), rowCount());
//        m_coordinates.append(coordinate);
//        endInsertRows();
//    }

//    int rowCount(const QModelIndex &parent = QModelIndex()) const override{
//        Q_UNUSED(parent)
//        return m_coordinates.count();
//    }

//    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
//        if (index.row() < 0 || index.row() >= m_coordinates.count())
//            return QVariant();
//        if(role== MarkerModel::positionRole)
//            return QVariant::fromValue(m_coordinates[index.row()]);
//        return QVariant();
//    }

//    QHash<int, QByteArray> roleNames() const override{
//        QHash<int, QByteArray> roles;
//        roles[positionRole] = "position";
//        return roles;
//    }

//private:
//    QList<QGeoCoordinate> m_coordinates;
//};

//#endif // MARKERMODEL_H

#ifndef MARKERMODEL_H
#define MARKERMODEL_H

#include <QAbstractListModel>
#include <QGeoCoordinate>

class MarkerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate current READ current NOTIFY currentChanged)
public:
    enum MarkerRoles{
        PositionRole = Qt::UserRole + 1000,
    };
    explicit MarkerModel(QObject *parent = nullptr);
    void moveMarker(const QGeoCoordinate & coordinate);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int maxMarkers() const;
    void setMaxMarkers(int maxMarkers=0);
    QGeoCoordinate current() const;
    void change(int id, const QGeoCoordinate & coordinate);
signals:
    void currentChanged();
private:
    void insert(int row, const QGeoCoordinate & coordinate);
    void removeLastMarker();
    QList<QGeoCoordinate> m_markers;
    QGeoCoordinate m_current;
    int m_maxMarkers;
};

#endif // MARKERMODEL_H
