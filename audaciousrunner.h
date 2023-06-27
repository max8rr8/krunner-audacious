#pragma once

#include <KRunner/AbstractRunner>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QReadWriteLock>
#include <QString>
#include <QVector>

class AudaciousRunner : public Plasma::AbstractRunner {
  Q_OBJECT

public:
  AudaciousRunner(QObject *parent, const KPluginMetaData &data,
                  const QVariantList &args);

  void match(Plasma::RunnerContext &context) override;
  void run(const Plasma::RunnerContext &context,
           const Plasma::QueryMatch &match) override;

  enum RunnerAction { Jump, Volume };

  Q_ENUM(RunnerAction);

private:
  QString getSong(uint id);
  bool ensurePlaylist();

  void GeneratePlayMatches(QString &query, QList<Plasma::QueryMatch> &matches);
  void GenerateVolMatches(QString &query, QList<Plasma::QueryMatch> &matches);

protected:
  void init() override;

private:
  QDBusInterface m_dbus;
  
  QReadWriteLock m_lock;

  int m_playlistLen;
  int m_position;
};