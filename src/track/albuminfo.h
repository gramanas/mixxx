#pragma once

#include <QString>
#include <QUuid>

#include "track/replaygain.h"

#include "util/macros.h"


namespace mixxx {

class AlbumInfo final {
    // Album properties (in alphabetical order)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    artist,                    Artist)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzArtistId,       MusicBrainzArtistId)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzReleaseId,      MusicBrainzReleaseId)
    PROPERTY_SET_BYVAL_GET_BYREF(QUuid,      musicBrainzReleaseGroupId, MusicBrainzReleaseGroupId)
    PROPERTY_SET_BYVAL_GET_BYREF(ReplayGain, replayGain,                ReplayGain)
    PROPERTY_SET_BYVAL_GET_BYREF(QString,    title,                     Title)

public:
    AlbumInfo() = default;
    AlbumInfo(AlbumInfo&&) = default;
    AlbumInfo(const AlbumInfo&) = default;
    /*non-virtual*/ ~AlbumInfo() = default;

    AlbumInfo& operator=(AlbumInfo&&) = default;
    AlbumInfo& operator=(const AlbumInfo&) = default;

    // TODO(XXX): Remove after all new fields have been added to the library
    void resetUnsupportedValues();

    // Adjusts floating-point values to match their string representation
    // in file tags to account for rounding errors.
    void normalizeBeforeExport() {
        refReplayGain().normalizeBeforeExport();
    }
};

bool operator==(const AlbumInfo& lhs, const AlbumInfo& rhs);

inline
bool operator!=(const AlbumInfo& lhs, const AlbumInfo& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const AlbumInfo& arg);

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::AlbumInfo)
