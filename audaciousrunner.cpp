#include "audaciousrunner.h"
#include <QDBusReply>
#include <climits>
#include <qchar.h>
#include <qdbusreply.h>
#include <qnamespace.h>
#include <qpair.h>
#include <qreadwritelock.h>
#include <qstringliteral.h>
#include <qvariant.h>
#include <qvector.h>

K_PLUGIN_CLASS_WITH_JSON(AudaciousRunner, "audaciousrunner.json")

#include "audaciousrunner.moc"

const QString audaciousService = QStringLiteral("org.atheme.audacious");
const QString audaciousPath = QStringLiteral("/org/atheme/audacious");
const QString audaciousInterface = QStringLiteral("org.atheme.audacious");
const QString triggerWord = QStringLiteral("adcs");

AudaciousRunner::AudaciousRunner(QObject *parent, const KPluginMetaData &data)
    : AbstractRunner(parent, data)
    , m_dbus(audaciousService, audaciousPath, audaciousInterface)
{
    setObjectName(QStringLiteral("Audacious Runner"));

    QList<KRunner::RunnerSyntax> syntaxes;
    syntaxes << KRunner::RunnerSyntax(QStringLiteral("%1 :q:").arg(triggerWord), QStringLiteral("Controll audacious player."));

    setSyntaxes(syntaxes);
    setTriggerWords({triggerWord});
}

bool AudaciousRunner::ensurePlaylist() {
  QDBusReply<int> dbusLength = m_dbus.call(QStringLiteral("Length"));
  if (!dbusLength.isValid())
    return false;

  QDBusReply<uint> dbusPosition = m_dbus.call(QStringLiteral("Position"));
  if (!dbusPosition.isValid())
    return false;

  {
    QWriteLocker locker(&m_lock);
    m_playlistLen = dbusLength.value();
    m_position = dbusPosition.value();
  }

  return true;
}

void AudaciousRunner::init() { reloadConfiguration(); }

QString AudaciousRunner::getSong(uint id) {
  QDBusReply<QString> song = m_dbus.call(QStringLiteral("SongTitle"), id);

  if (!song.isValid()) {
    qDebug() << "Failed to get song:" << song.error();
  }
  return song;
}

void AudaciousRunner::match(KRunner::RunnerContext &context)
{
  if (!ensurePlaylist())
    return;

  QReadLocker locker(&m_lock);

  QString query = context.query();
  query = query.mid(triggerWord.length()).trimmed();

  QList<KRunner::QueryMatch> matches;

  GenerateVolMatches(query, matches);
  GeneratePlayMatches(query, matches);

  qDebug() << "Ok";
  context.addMatches(matches);
}

void AudaciousRunner::GeneratePlayMatches(QString &query, QList<KRunner::QueryMatch> &matches)
{
  int delta = 0;
  int left = m_playlistLen;

  while (matches.size() < 7 && left > 0) {
    int id = m_position + delta;
    if (id < 0)
      id += m_playlistLen;
    if (id >= m_playlistLen)
      id -= m_playlistLen;

    QString songName = getSong(id);

    if (songName.contains(query, Qt::CaseInsensitive)) {
      KRunner::QueryMatch match(this);
      match.setData(QList<QVariant>{RunnerAction::Jump, id});
      match.setId(QStringLiteral("audacious://play/") + QString::number(id));
      match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::High);
      match.setText(QStringLiteral("%1: %2")
                        .arg(QString::number(id + 1), 4, QLatin1Char('0'))
                        .arg(songName));
      match.setRelevance(1 - 0.01 * matches.size());
      matches.append(match);
    }

    delta = -delta;
    if (delta >= 0)
      delta++;
    left--;
  }
}

void AudaciousRunner::GenerateVolMatches(QString &query, QList<KRunner::QueryMatch> &matches)
{
  if (query.size() < 2)
    return;

  QStringView volquery;

  if (query.startsWith(QStringLiteral("vol"))) {
    volquery = QStringView(query).mid(3).trimmed();
  } else if (query.at(0) == QLatin1Char('v') && !query.at(1).isLetter()) {
    volquery = QStringView(query).mid(1).trimmed();
  } else {
    return;
  }

  bool is_delta = volquery.startsWith(QLatin1Char('+')) ||
                  volquery.startsWith(QLatin1Char('-'));

  bool is_ok = false;
  int volume = volquery.toInt(&is_ok);
  if (!is_ok)
    return;

  QDBusReply<int> currentVolumeReply = m_dbus.call(QStringLiteral("Volume"));
  if (!currentVolumeReply.isValid())
    return;
  int currentVolume = currentVolumeReply.value();

  int next_volume = volume;
  if (is_delta)
    next_volume += currentVolume;

  KRunner::QueryMatch match(this);
  match.setData(QList<QVariant>{RunnerAction::Volume, next_volume});
  match.setId(QStringLiteral("audacious://volume/") +
              QString::number(next_volume));
  match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::High);
  match.setText(QStringLiteral("Set volume to: %2").arg(next_volume));
  match.setRelevance(1);
  matches.append(match);
}

void AudaciousRunner::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
  context.ignoreCurrentMatchForHistory();
  QList<QVariant> data = match.data().toList();
  RunnerAction action = data[0].value<RunnerAction>();

  if (action == RunnerAction::Jump) {
    int id = data[1].value<int>();
    m_dbus.call(QStringLiteral("Jump"), (uint)id);
  } else if (action == RunnerAction::Volume) {
    int volume = data[1].value<int>();
    m_dbus.call(QStringLiteral("SetVolume"), volume, volume);
  }
}
