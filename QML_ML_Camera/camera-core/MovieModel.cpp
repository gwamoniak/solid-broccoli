#include "MovieModel.h"

#include <QDateTime>
#include <QTime>
#include <QUrl>

#include "DatabaseManager.h"
#include "logger.h"

using namespace std;

namespace {
QString formatDuration(qint64 durationMs)
{
    const int totalSeconds = static_cast<int>(durationMs / 1000);
    return QStringLiteral("%1:%2")
        .arg(totalSeconds / 60, 2, 10, QLatin1Char('0'))
        .arg(totalSeconds % 60, 2, 10, QLatin1Char('0'));
}
}

MovieModel::MovieModel(DatabaseManager& db, QObject* parent) :
    QAbstractListModel(parent),
    m_sqlDB(db),
    m_vMovies(m_sqlDB.movieDao().movies())
{
    qDebug(logInfo()) << "MovieModel has been created!";
}

void MovieModel::addRecording(const QString& filePath, qint64 durationMs)
{
    // New recordings sort first (movies() orders by id DESC), so insert at row 0.
    beginInsertRows(QModelIndex(), 0, 0);
    unique_ptr<Movie> movie(new Movie(QUrl::fromLocalFile(filePath), durationMs,
                                      QDateTime::currentDateTime().toString(Qt::ISODate)));
    m_sqlDB.movieDao().addMovie(*movie);
    m_vMovies->insert(m_vMovies->begin(), std::move(movie));
    endInsertRows();
    qDebug(logInfo()) << "Recording registered:" << filePath;
}

void MovieModel::rename(int row, const QString& _name)
{
    const QModelIndex idx = index(row);
    if (!isIndexValid(idx)) {
        return;
    }
    Movie& movie = *m_vMovies->at(row);
    movie.set_sName(_name);
    m_sqlDB.movieDao().updateMovie(movie);
    emit dataChanged(idx, idx);
}

int MovieModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_vMovies->size();
}

QVariant MovieModel::data(const QModelIndex& _index, int role) const
{
    if (!isIndexValid(_index)) {
        return QVariant();
    }
    const Movie& movie = *m_vMovies->at(_index.row());

    switch (role) {
        case DBRoles::IdRole:
            return movie._nMovieID();

        case DBRoles::UrlRole:
            return movie._FileUrl();

        case DBRoles::FilePathRole:
            return movie._FileUrl().toLocalFile();

        case DBRoles::NameRole:
        case Qt::DisplayRole:
            return movie._sName();

        case DBRoles::DurationRole:
            return formatDuration(movie._nDurationMs());

        case DBRoles::CreatedAtRole:
            return movie._sCreatedAt();

        default:
            return QVariant();
    }
}

bool MovieModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0
            || row >= rowCount()
            || count < 0
            || (row + count) > rowCount()) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    int countLeft = count;
    while (countLeft--) {
        const Movie& movie = *m_vMovies->at(row + countLeft);
        m_sqlDB.movieDao().removeMovie(movie._nMovieID());
    }
    m_vMovies->erase(m_vMovies->begin() + row,
                     m_vMovies->begin() + row + count);
    endRemoveRows();
    return true;
}

QHash<int, QByteArray> MovieModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DBRoles::IdRole] = "id";
    roles[DBRoles::UrlRole] = "url";
    roles[DBRoles::FilePathRole] = "filepath";
    roles[DBRoles::NameRole] = "name";
    roles[DBRoles::DurationRole] = "duration";
    roles[DBRoles::CreatedAtRole] = "createdAt";
    return roles;
}

bool MovieModel::isIndexValid(const QModelIndex& _index) const
{
    return _index.isValid() && _index.row() < rowCount();
}
