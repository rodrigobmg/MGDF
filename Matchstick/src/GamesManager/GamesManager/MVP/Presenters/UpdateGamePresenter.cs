﻿using System;
using System.Collections.Generic;
using System.Configuration;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using MGDF.GamesManager.Common;
using MGDF.GamesManager.Common.Extensions;
using MGDF.GamesManager.Common.Framework;
using MGDF.GamesManager.Controls;
using MGDF.GamesManager.Model;
using MGDF.GamesManager.Model.Entities;
using MGDF.GamesManager.MVP.Views;

namespace MGDF.GamesManager.MVP.Presenters
{
    class UpdateGamePresenter: PresenterBase<IProgressView>
    {
        private string _gameUpdate;
        private string _gameUpdateHash;
        private string _frameworkUpdate;
        private string _frameworkUpdateHash;

        private Thread _workerThread;

        private object _lock = new object();
        private FileDownloader _currentDownloader;
        private LongRunningTask _currentTask;
        private Timer _progressTimer;

        public UpdateGamePresenter(string gameUpdate,string gameUpdateHash,string frameworkUpdate,string frameworkUpdateHash)
            : base(ViewFactory.Current.CreateView<IProgressView>())
        {
            _gameUpdate = gameUpdate;
            _gameUpdateHash = gameUpdateHash;
            _frameworkUpdate = frameworkUpdate;
            _frameworkUpdateHash = frameworkUpdateHash;

            View.Closed += View_Closed;
            View.OnCancel += View_OnCancel;

            View.AllowCancel = false;
            View.Title = "Updating " + Game.Current.Name;
            View.Details = "Updating " + Game.Current.Name + ", please wait...";
            View.ShowProgress(0, 500);

            _workerThread = new Thread(DoWork);
            _workerThread.Start();

            _progressTimer = new Timer(OnProgressUpdate,null,0,1000);
        }

        private void OnProgressUpdate(object state)
        {
            lock (_lock)
            {
                if (_currentTask != null)
                {
                    View.Invoke(() => View.ShowProgress(_currentTask.Progress, _currentTask.Total));
                }
            }
        }

        void View_OnCancel(object sender, EventArgs e)
        {
            lock (_lock)
            {
                if (_currentDownloader!=null) _currentDownloader.Cancel();
            }
        }

        private void View_Closed(object sender, EventArgs e)
        {
            _progressTimer.Dispose();
            if (_workerThread != null && _workerThread.ThreadState == ThreadState.Running) _workerThread.Abort();
        }

        private void DoWork()
        {
            try
            {
                Resources.InitUpdaterDirectories();

                if (_frameworkUpdate!=null)
                {
                    string frameworkFile = Path.Combine(Resources.DownloadsDir, "framework.zip");
                    try
                    {
                        lock (_lock)
                        {
                            Logger.Current.Write(LogInfoLevel.Info, "Downloading framework update...");
                            _currentDownloader = new FileDownloader(_frameworkUpdate, frameworkFile, _frameworkUpdateHash, null);
                            _currentTask = _currentDownloader;
                            View.Invoke(()=>
                            {
                                View.Details = "Downloading MGDF framework update...";
                                View.AllowCancel = true;
                            });
                        }
                        var result = _currentDownloader.Start();

                        lock (_lock)
                        {
                            _currentDownloader = null;
                            View.Invoke(()=>View.AllowCancel = false);
                        }

                        if (result==LongRunningTaskResult.Cancelled)
                        {
                            _workerThread = null;
                            View.Invoke(CloseView);
                            return;
                        }
                        else if (result==LongRunningTaskResult.Error)
                        {
                            //show an error message, though we may still be able to download a game update, so don't bail out yet.
                            ViewFactory.Current.CreateView<IMessage>().Show("Failed to download MGDF framework update", "Download failed");
                        }
                        else
                        {

                            lock (_lock)
                            {
                                Logger.Current.Write(LogInfoLevel.Info, "Installing framework update...");
                                //success, now try to install the downloaded update
                                View.Invoke(() => View.Details = "Installing MGDF framework update...");
                                _currentTask = new FrameworkUpdater(frameworkFile);
                            }
                            _currentTask.Start();
                        }
                    }
                    finally
                    {
                        var file = FileSystem.Current.GetFile(frameworkFile);
                        if (file.Exists) file.DeleteWithTimeout();
                    }
                }

                if (_gameUpdate != null)
                {
                    string gameUpdateFile = Path.Combine(Resources.DownloadsDir, "update.zip");
                    try
                    {
                        lock (_lock)
                        {
                            Logger.Current.Write(LogInfoLevel.Info, "Downloading game update...");
                            _currentDownloader = new GameDownloader(Game.Current, _gameUpdate, gameUpdateFile, _gameUpdateHash, GetUpdateCredentials);
                            _currentTask = _currentDownloader;
                            View.Invoke(() =>
                            {
                                View.Details = "Downloading "+Game.Current.Name+" update...";
                                View.AllowCancel = true;
                            });
                        }
                        var result = _currentDownloader.Start();

                        lock (_lock)
                        {
                            _currentDownloader = null;
                            View.Invoke(() => View.AllowCancel = false);
                        }

                        if (result == LongRunningTaskResult.Cancelled)
                        {
                            _workerThread = null;
                            View.Invoke(CloseView);
                            return;
                        }
                        else if (result == LongRunningTaskResult.Error)
                        {
                            ViewFactory.Current.CreateView<IMessage>().Show("Failed to download " + Game.Current.Name + " update", "Download failed");
                            _workerThread = null;
                            View.Invoke(CloseView);
                            return;
                        }
                        else
                        {
                            using (var gameInstall = new GameInstall(gameUpdateFile))
                            {
                                lock (_lock)
                                {
                                    Logger.Current.Write(LogInfoLevel.Info, "Installing framework update...");
                                    //success, now try to apply the downloaded update
                                    View.Invoke(() => View.Details = "Installing " + Game.Current.Name + " update...");
                                    _currentTask = new GameUpdater(gameInstall);
                                }
                                _currentTask.Start();
                            }

                            //now if we're auto installing on update, update the registry/desktop icons etc..
                            if (Config.Current.AutoRegisterOnUpdate)
                            {
                                lock (_lock)
                                {
                                    Logger.Current.Write(LogInfoLevel.Info, "Registering game update...");
                                    _currentTask = new GameRegistrar(true, Game.Current);
                                }
                                _currentTask.Start();
                            }
                        }
                    }
                    finally
                    {
                        var file = FileSystem.Current.GetFile(gameUpdateFile);
                        if (file.Exists) file.DeleteWithTimeout();
                    }
                }
            }
            catch (ThreadAbortException)
            {
                
            }
            catch (Exception ex)
            {
                View.Invoke(() => Program.ShowUnhandledError(ex));
            }

            _workerThread = null;
            View.Invoke(CloseView);
        }

        private bool GetUpdateCredentials(GetCredentialsEventArgs args)
        {
            GetCredentialsPresenter presenter = null;
            View.Invoke(() =>
            {
                presenter = new GetCredentialsPresenter(Game.Current, args);
                presenter.ShowView(View);
            });
            return presenter.OK;
        }
    }
}