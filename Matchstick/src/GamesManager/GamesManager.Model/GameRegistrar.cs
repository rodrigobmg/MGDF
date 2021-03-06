﻿
using System;
using System.Collections.Generic;
using System.IO;
using MGDF.GamesManager.Common;
using MGDF.GamesManager.Common.Framework;
using MGDF.GamesManager.Model.Entities;
using MGDF.GamesManager.Common.Extensions;
using File = MGDF.GamesManager.Common.Framework.File;
using InstalledProgramsHelper = MGDF.GamesManager.Model.Helpers.InstalledProgramsHelper;

namespace MGDF.GamesManager.Model
{
    public class GameRegistrar: LongRunningTask
    {
        private bool _register;
        private Game _game;

        public GameRegistrar(bool register,Game game)
        {
            _register = register;
            _game = game;

        }

        public override LongRunningTaskResult Start()
        {
            LongRunningTaskResult result = LongRunningTaskResult.Completed;
            try
            {
                CreateSystemIcon();
                AddToInstalledPrograms();
                AddToGamesExplorer();
                AddToStartMenu();
                AddDesktopShortcut();
				DependencyInstaller.Install();
            }
            catch (Exception ex)
            {
                Logger.Current.Write(ex,"Install failed");
                result = LongRunningTaskResult.Error;
            }

            return result;
        }

        private void CreateSystemIcon()
        {
            if (_register)
            {
                //make sure to create the games icon file if it hasn't been created already.
                if (FileSystem.Current.FileExists(FileSystem.Combine(Resources.GameBaseDir, Resources.GameIcon)))
                {
                    IconManager.Current.CreateIcon(_game.Name, FileSystem.Combine(Resources.GameBaseDir, Resources.GameIcon), FileSystem.Combine(Resources.GameBaseDir, Resources.GameSystemIcon));
                }
            }
        }

        private void AddToGamesExplorer()
        {
            //add the game to the game explorer if we're on vista/win7 and the Game has a gdf file
            if (FileSystem.Current.FileExists(Resources.GameDefinitionFileBin()))
            {
                try
                {
                    if (GameExplorer.Current.IsInstalled(Resources.GameDefinitionFileBin()))
                    {
                        GameExplorer.Current.UninstallGame(Resources.GameDefinitionFileBin());
                    }

                    if (_register)
                    {
                        Guid instanceId = GameExplorer.Current.InstallGame(
                            Resources.GameDefinitionFileBin(),
                            EnvironmentSettings.Current.AppDirectory,
                            Resources.GamesManagerExecutable,
                            Resources.GamesManagerBootArguments(string.Empty, string.Empty, string.Empty, string.Empty));

                        //add homepage link
                        if (!string.IsNullOrEmpty(_game.Homepage))
                        {
                            GameExplorer.Current.AddGameTask(instanceId, TaskCount.One, "Homepage", _game.Homepage, string.Empty);
                        }
                    }
                }
                catch (Exception ex)
                {
                    Logger.Current.Write(ex, "Unable to add game to games explorer");
                }
            }
        }

        private void AddDesktopShortcut()
        {
            string shortcutPath = Path.Combine(EnvironmentSettings.Current.CommonDesktopDirectory, _game.Name + ".lnk");
            if (ShortcutManager.Current.HasShortcut(shortcutPath))
            {
                ShortcutManager.Current.DeleteShortcut(shortcutPath);
            }

            if (_register)
            {
                ShortcutManager.Current.CreateShortcut(shortcutPath, Resources.GamesManagerExecutable, Resources.GamesManagerBootArguments(string.Empty, string.Empty, string.Empty, string.Empty), Resources.GameSystemIconFile());
            }
        }

        private void AddToStartMenu()
        {
            var startMenuFolder = FileSystem.Current.GetDirectory(Path.Combine(EnvironmentSettings.Current.CommonStartMenuDirectory, _game.DeveloperName));
            if (_register)
            {
                if (!startMenuFolder.Exists)
                {
                    startMenuFolder.Create();
                }
            }

            string shortcutPath = Path.Combine(startMenuFolder.FullName, _game.Name + ".lnk");
            if (ShortcutManager.Current.HasShortcut(shortcutPath))
            {
                ShortcutManager.Current.DeleteShortcut(shortcutPath);
            }

            if (_register)
            {
                ShortcutManager.Current.CreateShortcut(shortcutPath, Resources.GamesManagerExecutable, Resources.GamesManagerBootArguments(string.Empty, string.Empty, string.Empty, string.Empty), Resources.GameSystemIconFile());
            }
            else
            {
                if (startMenuFolder.GetFiles("*.*").Count==0)
                {
                    startMenuFolder.Delete();
                }
            }
        }

        private void AddToInstalledPrograms()
        {
            if (_register)
            {
                InstalledProgramsHelper.AddToInstalledPrograms(_game);
            }
            else
            {
                InstalledProgramsHelper.RemoveFromInstalledPrograms(_game.Uid);
            }
        }
    }
}