#ifndef MIXXX_CRATEFEATURE_H
#define MIXXX_CRATEFEATURE_H

#include <QList>
#include <QAction>
#include <QVariant>
#include <QUrl>
#include <QIcon>
#include <QPersistentModelIndex>
#include <QPoint>
#include <QStackedWidget>

#include "library/features/crates/cratetablemodel.h"
#include "library/features/crates/cratetreemodel.h"
#include "library/features/crates/cratemanager.h"

#include "library/libraryfeature.h"
#include "library/treeitemmodel.h"

#include "track/track.h"

#include "preferences/usersettings.h"

// forward declaration(s)
class Library;
class TrackCollection;

class CrateFeature : public LibraryFeature {
    Q_OBJECT
  public:
    CrateFeature(UserSettingsPointer pConfig,
                 Library* pLibrary,
                 QObject* parent,
                 TrackCollection* pTrackCollection);
    virtual ~CrateFeature() override;

    QVariant title() override;
    QString getIconPath() override;
    QString getSettingsName() const override;
    bool isSinglePane() const override;

    bool dragMoveAccept(QUrl url) override;
    bool dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                         QObject* pSource) override;
    bool dragMoveAcceptChild(const QModelIndex& index, QUrl url) override;

    parented_ptr<QWidget> createPaneWidget(KeyboardEventFilter* pKeyboard,
                                           int paneId, QWidget* parent) override;

    QPointer<TreeItemModel> getChildModel() override;

  signals:
    void analyzeTracks(QList<TrackId>);

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void invalidateChild() override;
    void onRightClick(const QPoint& globalPos) override;
    void onRightClickChild(const QPoint& globalPos, const QModelIndex& index) override;
    void slotCreateCrate();
    void slotCreateChildCrate();

  private slots:
    void slotDeleteCrate();
    void slotRenameCrate();
    void slotDuplicateCrate();
    void slotAutoDjTrackSourceChanged();
    void slotToggleCrateLock();
    void slotImportPlaylist();
    void slotImportPlaylistFile(const QString &playlist_file);
    void slotCreateImportCrate();
    void slotExportPlaylist();
    // Copy all of the tracks in a crate to a new directory (like a thumbdrive).
    void slotExportTrackFiles();
    void slotAnalyzeCrate();
    void slotCrateTableChanged(CrateId crateId);
    void slotCrateContentChanged(CrateId crateId);
    void htmlLinkClicked(const QUrl& link);
    void slotTrackSelected(TrackPointer pTrack);
    void slotResetSelectedTrack();
    void slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds);
    // moves the selected crate and it's subtree under another crate
    void slotMoveSubtreeToCrate(const Crate& destinationCrate);

  private:
    void initActions();
    void connectLibrary(Library* pLibrary);
    void connectCrateManager();

    bool activateCrate(CrateId crateId);
    void generateSummaries();

    std::unique_ptr<TreeItem> newTreeItemForCrateSummary(
            const CrateSummary& crateSummary);
    void updateTreeItemForCrateSummary(
            TreeItem* pTreeItem,
            const CrateSummary& crateSummary) const;

    void rebuildChildModel(CrateId selectedCrateId = CrateId());
    void updateChildModel(const QSet<CrateId>& updatedCrateIds);

    CrateId crateIdFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromCrateId(CrateId crateId) const;

    bool readLastRightClickedCrate(Crate* pCrate) const;

    QString formatRootViewHtml() const;

    const QIcon m_cratesIcon;
    const QIcon m_lockedCrateIcon;

    QPointer<CrateTableModel> getTableModel(int paneId);
    void showBrowse(int paneId);
    void showTable(int paneId);

    void toggleRecursion();

    TrackCollection* m_pTrackCollection;
    CrateManager* m_pCrates;

    CrateTableModel* m_pCrateTableModel;

    QModelIndex m_lastRightClickedIndex;
    TrackPointer m_pSelectedTrack;

    // FIXME(XXX): std::unique_ptr is wrong! Qt takes ownership
    // of these actions. Should be replaced with the appropriate
    // variant of parented_ptr as soon as it becomes available.
    // See also: https://github.com/mixxxdj/mixxx/pull/1161
    std::unique_ptr<QAction> m_pCreateCrateAction;
    std::unique_ptr<QAction> m_pCreateChildCrateAction;
    std::unique_ptr<QAction> m_pDeleteCrateAction;
    std::unique_ptr<QAction> m_pRenameCrateAction;
    std::unique_ptr<QAction> m_pLockCrateAction;
    std::unique_ptr<QAction> m_pDuplicateCrateAction;
    std::unique_ptr<QAction> m_pAutoDjTrackSourceAction;
    std::unique_ptr<QAction> m_pImportPlaylistAction;
    std::unique_ptr<QAction> m_pCreateImportPlaylistAction;
    std::unique_ptr<QAction> m_pExportPlaylistAction;
    std::unique_ptr<QAction> m_pExportTrackFilesAction;
    std::unique_ptr<QAction> m_pAnalyzeCrateAction;
    std::unique_ptr<QAction> m_pToggleRecursionAction;

    std::unique_ptr<QMenu> m_pMoveCrateMenu;


    std::unique_ptr<CrateTreeModel> m_pChildModel;

    QHash<int, QPointer<QStackedWidget> > m_panes;
    QHash<int, QPointer<CrateTableModel> > m_crateTableModel;
    QHash<int,int> m_idBrowse;
    QHash<int,int> m_idTable;
    QHash<int, QPersistentModelIndex> m_lastClickedIndex;
};


#endif // MIXXX_CRATEFEATURE_H
