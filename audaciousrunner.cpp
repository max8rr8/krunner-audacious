#include "audaciousrunner.h"
#include <QDBusReply>
#include <climits>
#include <krunner/querymatch.h>
#include <qdbusreply.h>
#include <qnamespace.h>
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

AudaciousRunner::AudaciousRunner(QObject *parent, const KPluginMetaData &data,
                                 const QVariantList &args)
    : AbstractRunner(parent, data, args),
      m_dbus(audaciousService, audaciousPath, audaciousInterface) {

  setObjectName(QStringLiteral("Audacious Runner"));
  setPriority(LowPriority);

  QList<Plasma::RunnerSyntax> syntaxes;
  syntaxes << Plasma::RunnerSyntax(
      QStringLiteral("%1 :q:").arg(triggerWord),
      QStringLiteral("Controll audacious player."));

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

void AudaciousRunner::match(Plasma::RunnerContext &context) {
  if (!ensurePlaylist())
    return;

  QReadLocker locker(&m_lock);

  QString term = context.query();
  term = term.mid(triggerWord.length()).trimmed();

  QList<Plasma::QueryMatch> matches;
  int delta = 0;
  int left = m_playlistLen;

  while (matches.size() < 7 && left > 0) {
    int id = m_position + delta;
    if (id < 0)
      id += m_playlistLen;
    if (id >= m_playlistLen)
      id -= m_playlistLen;

    QString songName = getSong(id);

    if (songName.contains(term, Qt::CaseInsensitive)) {
      Plasma::QueryMatch match(this);
      match.setData(QList<QVariant>{RunnerAction::Jump, id});
      match.setId(QStringLiteral("audacious://play/") + QString::number(id));
      match.setType(Plasma::QueryMatch::CompletionMatch);
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
  context.addMatches(matches);
}

void AudaciousRunner::run(const Plasma::RunnerContext &context,
                          const Plasma::QueryMatch &match) {
  context.ignoreCurrentMatchForHistory();
  QList<QVariant> data = match.data().toList();
  RunnerAction action = data[0].value<RunnerAction>();

  if (action == RunnerAction::Jump) {
    int id = data[1].value<int>();
    m_dbus.call(QStringLiteral("Jump"), (uint)id);
  }
}
