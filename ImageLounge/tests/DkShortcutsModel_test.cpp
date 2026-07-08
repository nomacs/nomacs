
#include "DkDialog.h"

#include "DkTestFixture.h"

class ShortcutsModel : public ActionsTest
{
};

TEST_F(ShortcutsModel, Init)
{
#ifdef WITH_PLUGINS
    auto &am = nmc::DkActionManager::instance();
    nmc::DkPluginActionManager *pm = am.pluginActionManager();
    EXPECT_TRUE(pm != nullptr);
    // if this fails QT_PLUGIN_PATH needs to be <...>/nomacs-plugins
    EXPECT_TRUE(pm->pluginActions().count() > 0);
#endif

    {
        const nmc::DkShortcutsModel model;
        EXPECT_TRUE(model.checkState());

        auto amActions = nmc::DkActionManager::instance().allActions();

        // check actions the model pulled from
        const auto actionGroups = model.actions();
        int actionCount = 0;
        for (auto &list : actionGroups) {
            actionCount += list.count();
            for (auto *action : list) {
                // AM actions do not include all plugin actions due to lazy-init of plugins
                if (action->objectName().startsWith("plugin_")) {
                    continue;
                }
                EXPECT_TRUE(amActions.contains(action))
                    << "model action not found in action manager: " << qPrintable(action->objectName());
            }
        }

        // check the model is correctly initialized
        // the first level of the model is the category
        // the second level of the model contains all bindable actions
        int modelCount = 0;
        for (int row = 0; row < model.rowCount(); ++row) {
            QModelIndex index = model.index(row, 0, {});
            ASSERT_TRUE(index.isValid());
            modelCount += model.rowCount(index);
        }
        EXPECT_EQ(modelCount, actionCount) << "model count does not match action count";
    }

    {
        nmc::DkShortcutsModel model;
        const QKeySequence keySeq{Qt::CTRL | Qt::Key_A};
        const auto keySeqStr = keySeq.toString(QKeySequence::NativeText);
        const auto groupIndex = model.index(0, 0, {});
        // get the first two shortcuts and see if we can make a duplicate binding
        auto shortcut1 = model.index(0, 1, groupIndex);
        auto shortcut2 = model.index(1, 1, groupIndex);
        EXPECT_TRUE(model.setData(shortcut1, keySeq));
        EXPECT_TRUE(model.data(shortcut1) == keySeq);
        EXPECT_TRUE(model.setData(shortcut2, keySeq));
        EXPECT_TRUE(model.data(shortcut2) == keySeq);
        EXPECT_TRUE(model.data(shortcut1) == QKeySequence{}); // duplicate removed

        // test the duplicate signal
        bool signalReceived = false;
        QObject::connect(&model, &nmc::DkShortcutsModel::duplicateSignal, &model, [&](const QString &info) {
            EXPECT_TRUE(info.contains(keySeqStr)) << "duplicates message missing keysequence text:\n"
                                                  << "{" << qPrintable(info) << "}";
            signalReceived = true;
        });
        model.checkDuplicate(keySeq, nullptr);
        EXPECT_TRUE(signalReceived) << "duplicate signal not emitted";
    }

    {
        nmc::DkShortcutsModel model;
        const QKeySequence keySeq(Qt::CTRL | Qt::Key_A);
        const auto groupIndex = model.index(0, 0, {});
        auto name = model.index(0, 0, groupIndex);
        auto shortcut = model.index(0, 1, groupIndex);
        QAction *action = model.actions().at(0).at(0);
        EXPECT_TRUE(model.setData(shortcut, keySeq));
        qDebug() << "saving" << action->objectName() << model.data(name) << model.data(shortcut);
        EXPECT_TRUE(action->shortcut() != keySeq) << "action shortcut already updated";
        model.saveActions();
        EXPECT_TRUE(action->shortcut() == keySeq) << "action shortcut not updated";
        model.resetActions();
        EXPECT_TRUE(action->shortcut() == QKeySequence{}) << "action shortcut not cleared";
    }
}
