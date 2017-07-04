/*
* This file is part of libmeegomtp package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Santosh Puranik <santosh.puranik@nokia.com>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list
* of conditions and the following disclaimer. Redistributions in binary form must
* reproduce the above copyright notice, this list of conditions and the following
* disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may be
* used to endorse or promote products derived from this software without specific
* prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

// System headers
#include <QString>
#include <QRegExp>
#include <QUuid>
#include <QUrl>
/*
#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QSparqlConnection>
#include <QSparqlError>
#include <QSparqlQuery>
#include <QSparqlResult>
*/
// Local headers
#include "storagetracker.h"
#include "trace.h"

using namespace meegomtp1dot0;

// Static declarations
static const QString IRI_PREFIX = "file://";
static const QString RDF_TYPE = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
static QString getDateCreated (const QString&);
static QString getName (const QString&);
static QString getArtist (const QString&);
static QString getWidth (const QString&);
static QString getHeight (const QString&);
static QString getDuration (const QString&);
static QString getTrack (const QString&);
static QString getGenre (const QString&);
static QString getUseCount (const QString&);
static QString getAlbumName (const QString&);
static QString getBitrateType (const QString&);
static QString getSampleRate (const QString&);
static QString getNbrOfChannels (const QString&);
static QString getAudioBitDepth (const QString&);
static QString getAudioWAVECodec (const QString&);
static QString getAudioBitRate (const QString&);
static QString getVideoFourCCCodec (const QString&);
static QString getVideoBitRate (const QString&);
static QString getFramesPerThousandSecs (const QString&);
static QString getDRMStatus (const QString&);


static void setDateCreated (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setName (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setArtist (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setWidth (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setHeight (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setDuration (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setDRMStatus (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setTrack (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setGenre (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setUseCount (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setAlbumName (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setBitrateType (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setSampleRate (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setNbrOfChannels (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setAudioBitDepth (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setAudioWAVECodec (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setAudioBitRate (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setVideoFourCCCodec (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setVideoBitRate (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);
static void setFramesPerThousandSecs (const QString& iri, QString& val, QStringList& domains, QString &extraInserts);

static void trackerQuery(const QString&, QVector<QStringList> &res);
static void trackerUpdateQuery(const QString&);
static void convertResultByTypeAndCode(const QString&, QString&, MTPDataType, MTPObjPropertyCode, QVariant&);
static QString getValAsString(const QVariant& propVal, MTPDataType type, MTPObjPropertyCode code);
static QString generateIriForTracker(const QString& path);
static void deletePlaylistByIri(const QString &iri);

static const QString MINER_DEST("org.freedesktop.Tracker1.Miner.Files");
static const QString MINER_PATH("/org/freedesktop/Tracker1/Miner/Files");
static const QString MINER_IF("org.freedesktop.Tracker1.Miner");

StorageTracker::StorageTracker()
{
    populateFunctionMap();
}

StorageTracker::~StorageTracker()
{

}

// Populates the lookup table with functions to fetch respective Object info.
void StorageTracker::populateFunctionMap()
{
}

static void convertResultByTypeAndCode(const QString& filePath, QString& res, MTPDataType type, MTPObjPropertyCode code, QVariant& convertedResult)
{
    switch(type)
    {
        case MTP_DATA_TYPE_STR:
            {
                if(MTP_OBJ_PROP_Name == code && (res == ""))
                {
                    int idx = filePath.lastIndexOf(QChar('/'));
                    res = filePath.mid(idx + 1);
                }
                convertedResult = QVariant::fromValue(res);
            }
            break;
        case MTP_DATA_TYPE_INT8:
        case MTP_DATA_TYPE_UINT8:
            {
                // Just true and false for now!
                if(res == "true")
                {
                    convertedResult = QVariant::fromValue((quint8)1);
                }
                else
                {
                    convertedResult = QVariant::fromValue((quint8)0);
                }
            }
            break;
        case MTP_DATA_TYPE_INT16:
            {
                // Remove any thing after the decimal point...
                res = res.section('.', 0, 0);
                qint16 val = res.toShort();
                convertedResult = QVariant::fromValue(val);
            }
            break;
        case MTP_DATA_TYPE_UINT16:
            {
                if( MTP_OBJ_PROP_DRM_Status == code )
                {
                    if( "true" == res )
                    {
                        convertedResult = QVariant(0x0001);
                    }
                    else
                    {
                        convertedResult = QVariant(0x0000);
                    }
                }
                else
                {
                    // Remove any thing after the decimal point...
                    res = res.section('.', 0, 0);
                    quint16 val = res.toUShort();
                    convertedResult = QVariant::fromValue(val);
                }
            }
            break;
        case MTP_DATA_TYPE_INT32:
            {
                // Remove any thing after the decimal point...
                res = res.section('.', 0, 0);
                qint32 val = res.toLong();
                convertedResult = QVariant::fromValue(val);
            }
            break;
        case MTP_DATA_TYPE_UINT32:
            {
                // Remove any thing after the decimal point...
                res = res.section('.', 0, 0);
                quint32 val = res.toULong();
                convertedResult = QVariant::fromValue(val);
            }
            break;
        case MTP_DATA_TYPE_INT64:
            {
                // Remove any thing after the decimal point...
                res = res.section('.', 0, 0);
                qint64 val = res.toLongLong();
                convertedResult = QVariant::fromValue(val);
            }
            break;
        case MTP_DATA_TYPE_UINT64:
            {
                // Remove any thing after the decimal point...
                res = res.section('.', 0, 0);
                quint64 val = res.toULongLong();
                convertedResult = QVariant::fromValue(val);
            }
            break;
        // The below types are not currently used in tracker queries
        case MTP_DATA_TYPE_INT128:
        case MTP_DATA_TYPE_UINT128:
        case MTP_DATA_TYPE_AINT8:
        case MTP_DATA_TYPE_AUINT8:
        case MTP_DATA_TYPE_AINT16:
        case MTP_DATA_TYPE_AUINT16:
        case MTP_DATA_TYPE_AINT32:
        case MTP_DATA_TYPE_AUINT32:
        case MTP_DATA_TYPE_AINT64:
        case MTP_DATA_TYPE_AUINT64:
        case MTP_DATA_TYPE_AINT128:
        case MTP_DATA_TYPE_AUINT128:
        case MTP_DATA_TYPE_UNDEF:
        default:
            break;
    }

    if(MTP_OBJ_PROP_Duration == code)
    {
        quint32 v = convertedResult.value<quint32>();
        v = v * 1000;
        convertedResult = QVariant::fromValue(v);
    }
}

static QString getValAsString(const QVariant& propVal, MTPDataType type, MTPObjPropertyCode code)
{
    QString ret;
    switch(type)
    {
        case MTP_DATA_TYPE_INT8:
        case MTP_DATA_TYPE_UINT8:
            {
                // Only booleans for now!
                quint8 num = propVal.value<quint8>();
                if(0 == num)
                {
                    ret = "false";
                }
                else
                {
                    ret = "true";
                }
            }
            break;
        case MTP_DATA_TYPE_INT16:
        case MTP_DATA_TYPE_INT32:
            {
               long num = propVal.value<long>();
               ret = QString::number(num);
            }
            break;
        case MTP_DATA_TYPE_UINT16:
        case MTP_DATA_TYPE_UINT32:
            {
               ulong num = propVal.value<ulong>();
               if(MTP_OBJ_PROP_Duration == code)
               {
                   num /= 1000;
               }
               ret = QString::number(num);
               if( MTP_OBJ_PROP_DRM_Status == code )
               {
                   if( "1" == ret )
                   {
                       ret = "true";
                   }
                   else
                   {
                       ret = "false";
                   }
               }
            }
            break;
        case MTP_DATA_TYPE_INT64:
            {
                qlonglong num = propVal.value<qlonglong>();
                ret = QString::number(num);
            }
            break;
        case MTP_DATA_TYPE_UINT64:
            {
                qulonglong num = propVal.value<qulonglong>();
                ret = QString::number(num);
            }
            break;
        case MTP_DATA_TYPE_STR:
            {
                ret = propVal.value<QString>();
                if(MTP_OBJ_PROP_Date_Created == code)
                {
                    // Add : after +/-, for the timezone, if any
                    int idx = ret.lastIndexOf(QRegExp("\\+|\\-"));
                    if(-1 != idx)
                    {
                        ret.insert(idx + 3, ":");
                    }

                    // Covert date from MTP representation to ISO 8601
                    // Remove any tenths of seconds first...
                    ret.replace(QRegExp("\\.[0-9]"), "");
                    // Add -'s and :'s
                    ret.insert(4, "-");
                    ret.insert(7, "-");
                    ret.insert(13, ":");
                    ret.insert(16, ":");
                }
            }
            break;
    // The below types are not currently supported for writing to tracker
        case MTP_DATA_TYPE_INT128:
        case MTP_DATA_TYPE_UINT128:
        case MTP_DATA_TYPE_AINT8:
        case MTP_DATA_TYPE_AUINT8:
        case MTP_DATA_TYPE_AINT16:
        case MTP_DATA_TYPE_AUINT16:
        case MTP_DATA_TYPE_AINT32:
        case MTP_DATA_TYPE_AUINT32:
        case MTP_DATA_TYPE_AINT64:
        case MTP_DATA_TYPE_AUINT64:
        case MTP_DATA_TYPE_AINT128:
        case MTP_DATA_TYPE_AUINT128:
        default:
            MTP_LOG_WARNING("Unsupported data type in tracker write::" << type);
            break;
    }
    // Escape any 's in the value, as the query encloses the values in single quotes
    ret.replace(QRegExp("'"), "\\'");
    return ret;
}

QString StorageTracker::buildQuery(const QString &filePath, QList<MTPObjPropDescVal> &propValList)
{   return QString();
}

QString StorageTracker::buildMassQuery(const QString &path,
                                       const QList<const MtpObjPropDesc *> &properties)
{
        return QString();
}

bool StorageTracker::getPropVals(const QString &filePath, QList<MTPObjPropDescVal> &propValList)
{
    bool ret = false;
    return ret;
}

void StorageTracker::getChildPropVals(const QString& parentPath,
        const QList<const MtpObjPropDesc *>& properties,
        QMap<QString, QList<QVariant> > &values)
{
    return;

}

// Fetch the property value for the object referenced by the iri (ex: file:///home/user/MyDocs/1.mp3).
// Caller must free the pResult by calling storage_tracker_free_tracker_result
bool StorageTracker::getObjectProperty(const QString& path, MTPObjPropertyCode ePropertyCode, MTPDataType type, QVariant& result)
{
    return false;
}

QString StorageTracker::buildUpdateQuery(const QString &filePath, QList<MTPObjPropDescVal> &propValList)
{   return QString();
}

void StorageTracker::setPropVals(const QString &filePath, QList<MTPObjPropDescVal> &propValList)
{

}

bool StorageTracker::setObjectProperty(const QString& path, MTPObjPropertyCode ePropertyCode, MTPDataType type, const QVariant& propVal)
{
    return false;
}



static void trackerQuery(const QString& query, QVector<QStringList> &res)
{
}

static void trackerUpdateQuery(const QString& query)
{
}

static QString generateIriForTracker(const QString& path)
{
    QUrl url(IRI_PREFIX + path);
    QString iri = url.toEncoded();
    iri.replace(QRegExp("'"), "\\'");
    return iri;
}



void StorageTracker::ignoreNextUpdate(const QStringList &iris)
{
}

QString StorageTracker::savePlaylist(const QString &playlistPath, QStringList &entries)
{
    return QString("FAKE_PLAYLIST");
}

void StorageTracker::getPlaylists(QStringList &playlistPaths, QList<QStringList> &entries, bool getExisting /*= false*/)
{
}

bool StorageTracker::isPlaylistExisting(const QString &path)
{
    bool ret = false;

    return ret;
}

void StorageTracker::setPlaylistPath(const QString &name, const QString &path)
{

}

void StorageTracker::deletePlaylist(const QString &path)
{

}

static void deletePlaylistByIri(const QString &playlistUri)
{

}

void StorageTracker::movePlaylist(const QString &fromPath, const QString &toPath)
{
}

void StorageTracker::move(const QString &fromPath, const QString &toPath)
{
}

QString StorageTracker::generateIri(const QString &path)
{
    return generateIriForTracker(path);
}

bool StorageTracker::supportsProperty(MTPObjPropertyCode code) const
{
    return false;
}

void StorageTracker::copy(const QString &fromPath, const QString &toPath)
{
}

bool StorageTracker::isTrackerPropertySupported(const QString &property)
{
    return false;
}
