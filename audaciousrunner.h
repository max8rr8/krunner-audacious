#pragma once

#include <KRunner/AbstractRunner>
#include <KRunner/Action>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QReadWriteLock>
#include <QString>
#include <QVector>

class AudaciousRunner : public KRunner::AbstractRunner {
  Q_OBJECT

public:
  AudaciousRunner(QObject *parent, const KPluginMetaData &data);

  void match(KRunner::RunnerContext &context) override;
  void run(const KRunner::RunnerContext &context,
           const KRunner::QueryMatch &match) override;

  enum RunnerAction { Jump, Volume };

  Q_ENUM(RunnerAction);

private:
  QString getSong(uint id);
  bool ensurePlaylist();

  void GeneratePlayMatches(QString &query, QList<KRunner::QueryMatch> &matches);
  void GenerateVolMatches(QString &query, QList<KRunner::QueryMatch> &matches);

protected:
  void init() override;

private:
  QDBusInterface m_dbus;
  
  QReadWriteLock m_lock;

  int m_playlistLen;
  int m_position;
};