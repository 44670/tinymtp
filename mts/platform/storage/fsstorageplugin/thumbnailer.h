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

#ifndef THUMBNAILER_H
#define THUMBNAILER_H
#include <QObject>
#include <QString>
#include <QHash>
#include "thumbnailerproxy.h"

/// \brief The Thumbnailer class provides methods to generate representative
/// samples
/// The Thumbnailer class is used within the fsstorage plugin to generate
/// representative samples for objects. The class has one public method that is
/// used to requests for a thumbnail. Upon successful completion of the request,
/// a thumbnailReady signal is emitted. Upon error, the class simply logs the
/// error (Error handling is not very important for the representative sample
/// MTP property). Internally, the class uses the DBUS service exposed by
/// tumbler (http://live.gnome.org/ThumbnailerSpec).
namespace meegomtp1dot0
{
class Thumbnailer : public ThumbnailerProxy
{
    Q_OBJECT
    public:
        /// Constructor; Default and the only constructor.
        Thumbnailer();
        /// \brief Request a thumbnail.
        /// Use this method to request a thumbnail for the file at the given
        /// path. If the thumbnail for the given path is already present (in the
        /// system thumbnailer's cache), the path to the thumbnail is returned.
        /// If no thumbnail is found in the cache, this sends a DBUS request to
        /// the thumbnailer and returns immediately (with an empty return
        /// value). A signal is asynchronously emitted when the thumbnail is
        /// available.
        /// \param filePath [in] The absolute path of the file for which
        /// thumbnail is required.
        /// \param mimeType [in] The MIME type of the file specified in argument
        /// 1.
        /// \return Returns the absolute path of the thumbnail file, if
        /// available, else returns an empty string.
        QString requestThumbnail(const QString &filePath, const QString &mimeType);
    Q_SIGNALS:
        /// \brief Signal to indicate that thumbnail is now available.
        /// Thumbnailer emits this signal when the thumbnail for the path
        /// specified by the first argument is available.
        /// \param path [out] The absolute path of the file for which the
        /// thumbnail is available (The same path as requested in request).
        /// \see requestThumbnail
        void thumbnailReady(const QString &path);
    public Q_SLOTS:
        ///< This slot handles the Ready signal from the DBUS interface
        void slotThumbnailReady(uint, const QStringList&);
        ///< This slot handles the Started signal from the DBUS interface
        void slotRequestStarted(uint);
        ///< This slot handles the Finished signal from the DBUS interface
        void slotRequestFinished(uint);
        ///< This slot handles the Error signal from the DBUS interface
        void slotError(uint, const QStringList&, int, const QString&);
    private:
        ///< Checks if thumbnail for the requested path is already present in
        /// the system thumbnailer's cache.
        bool checkThumbnailPresent(const QString& filePath, QString& thumbPath);
        ///< Records the thumbnail request into the internal map
        void recordRequest(const QString&, uint);
        ///< A list of available thumbnail directories
        QStringList m_thumbnailDirs;
        ///< Internal map to keep track of thumbnail requests by handle
        QHash<uint, QString> m_requestMap;
        ///< Internal map to keep track of thumbnail requests by URI
        QHash<QString, uint> m_uriMap;
        ///< The maximum number of pending requests possible. Requests beyond
        /// this will be ignored.
        const qint32 MAX_REQ_MAP_SIZE;
        ///< The scheduler to use for the system thumbnailer. The class
        /// currently fixes this to "foreground".
        const QString m_scheduler;
        ///< The "flavor" of the thumbnail needed. The class currently fixes
        /// this to "normal".
        const QString m_flavor;
};
}
#endif // THUMBNAILER_H

