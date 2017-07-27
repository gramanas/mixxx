#ifndef MIXXX_CRATETABLEMODEL_H
#define MIXXX_CRATETABLEMODEL_H


#include "library/basesqltablemodel.h"

#include "library/crate/crateid.h"


class CrateTableModel : public BaseSqlTableModel {
    Q_OBJECT

  public:
    CrateTableModel(QObject* parent, TrackCollection* pTrackCollection);
    ~CrateTableModel() final;

    void selectCrate(
        Crate crate = Crate());
    CrateId selectedCrate() const {
        return m_selectedCrate;
    }

    bool addTrack(const QModelIndex &index, QString location);

    // From TrackModel
    bool isColumnInternal(int column) final;
    void removeTracks(const QModelIndexList& indices) final;
    // Returns the number of unsuccessful track additions
    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;
    CapabilitiesFlags getCapabilities() const final;

  private:
    CrateId m_selectedCrate;
    CrateManager* m_pCrates;
};


#endif // MIXXX_CRATETABLEMODEL_H
