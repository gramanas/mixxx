#include "util/db/sqllikewildcards.h"
#include "library/queryutil.h"
#include "library/features/crates/cratehierarchy.h"
#include "util/logger.h"

namespace {

const mixxx::Logger kLogger("CrateStorage");

} // anonymus namespace

void CrateHierarchy::initialize(const QSqlDatabase& database) {
    m_database = database;
}

uint CrateHierarchy::countCratesInClosure() const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT COUNT(*) FROM %1 "
        "WHERE %2 = %3").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID));
    if (query.execPrepared() && query.next()) {
        uint result = query.fieldValue(0).toUInt();
        DEBUG_ASSERT(!query.next());
        return result;
    } else {
        return 0;
    }
}

void CrateHierarchy::resetClosure() const {
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM %1").arg(
          CRATE_CLOSURE_TABLE));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}

bool CrateHierarchy::initClosure(CrateSelectResult crates) const {
    std::vector<CrateId> crateIds;

    //    CrateSelectResult crates(selectCrates());
    Crate crate;

    while (crates.populateNext(&crate)) {
        crateIds.push_back(crate.getId());
    }

    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 VALUES("
        ":parent, :child, 0)").arg(
          CRATE_CLOSURE_TABLE));
    if (!query.isPrepared()) {
        return false;
    }

    while (crateIds.size() != 0) {
        query.bindValue(":parent", crateIds.back());
        query.bindValue(":child", crateIds.back());
        if (!query.execPrepared()) {
            return false;
        }
        crateIds.pop_back();
    }

    return true;
}

void CrateHierarchy::resetPath() const {
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM %1").arg(
          CRATE_PATH_TABLE));
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }
}

bool CrateHierarchy::writeCratePaths(CrateId id, QString namePath, QString idPath) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 "
        "VALUES (:id, :idPath, :namePath)").arg(
          CRATE_PATH_TABLE));
    if (!query.isPrepared()) {
        return false;
    }
    query.bindValue(":id", id);
    query.bindValue(":namePath", namePath);
    query.bindValue(":idPath", idPath);
    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

bool CrateHierarchy::generateCratePaths(Crate crate) const {
    QSqlQuery query(m_database);

    query.prepare(
      QString(
        "SELECT p.%1, p.%2 FROM %3 "
        "JOIN %4 p ON %5 = p.%2 "
        "JOIN %4 c ON %6 = c.%2 "
        "where c.%1 = :childName and %7 != 0 "
        "ORDER BY %7 DESC").arg(
          CRATETABLE_NAME,
          CRATETABLE_ID,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID,
          CLOSURE_DEPTH));

    query.bindValue(":childName", crate.getName());
    query.setForwardOnly(true);
    QString namePath;
    QString idPath;

    if (query.exec()) {
        while (query.next()) {
            namePath = namePath + "/" + query.value(0).toString();
            idPath = idPath + "/" + query.value(1).toString();
        }
    } else {
        return false;
    }

    namePath = namePath + "/" + crate.getName();
    idPath = idPath + "/" + crate.getId().toString();

    return writeCratePaths(crate.getId(), namePath, idPath);
}

// slectCrates
bool CrateHierarchy::generateAllPaths(CrateSelectResult crates) const {
    //CrateSelectResult crates(selectCrates());
    Crate crate;
    while (crates.populateNext(&crate)) {
        generateCratePaths(crate);
    }

    return true;
}


bool CrateHierarchy::findParentAndChildIdFromPath(CrateId& parentId,
                                                CrateId& childId,
                                                const QString& idPath) const {
    QStringList ids = idPath.split("/", QString::SkipEmptyParts);

    // get the last item (childId)
    childId = CrateId(ids.back().toInt());
    if (ids.size() > 1) {
        // get the second to last item (parentId)
        parentId = CrateId(ids.at(ids.size() - 2));
    } else {
        // if there isn't one return false
        return false;
    }
    return true;
}


bool CrateHierarchy::initClosureForCrate(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1 "
        "VALUES(:parent, :child, 0)").arg(
          CRATE_CLOSURE_TABLE));

    if (!query.isPrepared()) {
        return false;
    }

    // closure dependacy
    query.bindValue(":parent", id);
    query.bindValue(":child", id);
    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

bool CrateHierarchy::insertIntoClosure(CrateId parent, CrateId child) const {
    FwdSqlQuery query(
      m_database, QString(
        "INSERT INTO %1(%2, %3, %4) "
        "SELECT p.%2, c.%3, p.%4 + c.%4 + 1 "
        "FROM %1 p, %1 c "
        "WHERE p.%3 = :parent AND c.%2 = :child").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_CHILDID,
          CLOSURE_DEPTH));

    if (!query.isPrepared()) {
        return false;
    }

    query.bindValue(":parent", parent);
    query.bindValue(":child", child);

    if (!query.execPrepared()) {
        return false;
    }

    return true;
}

void CrateHierarchy::deleteCrate(CrateId id) const {
    // TODO(gramanas) cratedeletion from the hierarchy must
    // be smart (delete crate with children)
    FwdSqlQuery query(
      m_database, QString(
        "DELETE FROM cratePath WHERE crateId = :id"));
    query.bindValue(":id", id);
    if (!query.isPrepared()) {
        return;
    }
    if (!query.execPrepared()) {
        return;
    }

    FwdSqlQuery query2(m_database, QString(
        "DELETE FROM crateClosure WHERE childId = :id"));
    query2.bindValue(":id", id);
    if (!query2.isPrepared()) {
        return;
    }
    if (!query2.execPrepared()) {
        return;
    }
}

QString CrateHierarchy::formatQueryForTrackIdsByCratePathLike(const QString& cratePathLike) const {
    FieldEscaper escaper(m_database);
    QString escapedArgument = escaper.escapeString(kSqlLikeMatchAll + cratePathLike + kSqlLikeMatchAll);

    return QString(
        "SELECT DISTINCT %1 FROM %2 "
        "JOIN %3 ON %4=%5 "
        "WHERE %6 LIKE %7 "
        "ORDER BY %1").arg(
            CRATETRACKSTABLE_TRACKID,
            CRATE_TRACKS_TABLE,
            CRATE_PATH_TABLE,
            PATHTABLE_CRATEID,
            CRATETRACKSTABLE_CRATEID,
            PATHTABLE_NAME_PATH,
            escapedArgument);

}

QString CrateHierarchy::getNamePathFromId(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "WHERE %3 = :id").arg(
          PATHTABLE_NAME_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_CRATEID));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
        return query.fieldValue(0).toString();
    }
    return QString();

}

bool CrateHierarchy::hasChildern(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT COUNT(*) FROM %1 "
        "WHERE %2 = :id AND %3 != 0").arg(
          CRATE_CLOSURE_TABLE,
          CLOSURE_PARENTID,
          CLOSURE_DEPTH));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
        return query.fieldValue(0).toUInt() != 0;
    }
    return false;
}

QStringList CrateHierarchy::collectIdPaths() const {
    QSqlQuery query(m_database);

    query.prepare(
      QString(
        "SELECT %1 FROM %2 "
        "ORDER BY %3").arg(
          PATHTABLE_ID_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_NAME_PATH));

    query.setForwardOnly(true);

    QString idPath;
    QStringList idPaths = {};

    if (query.exec()) {
        while (query.next()) {
            idPath = query.value(0).toString();
            idPaths.append(idPath);
        }
    } else {
        return idPaths; //returns empty list if it fails
    }

    return idPaths;
}

QStringList CrateHierarchy::tokenizeCratePath(CrateId id) const {
    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "WHERE %3 = :id").arg(
          PATHTABLE_NAME_PATH,
          CRATE_PATH_TABLE,
          PATHTABLE_CRATEID));

    query.bindValue(":id", id);
    if (query.execPrepared() && query.next()) {
            return query.fieldValue(0).toString().
                split("/", QString::SkipEmptyParts);
    }

    return QStringList();
}

QStringList CrateHierarchy::collectRootCrateNames() const {
    QStringList names;

    FwdSqlQuery query(
      m_database, QString(
        "SELECT %1 FROM %2 "
        "JOIN %3 ON %4 = %5 "
        "GROUP BY %5 "
        "HAVING COUNT(*) = 1").arg(
          CRATETABLE_NAME,
          CRATE_CLOSURE_TABLE,
          CRATE_TABLE,
          CRATETABLE_ID,
          CLOSURE_CHILDID));

    if (query.execPrepared())
        while (query.next()) {
            names << query.fieldValue(0).toString();
    }

    return names;
}
