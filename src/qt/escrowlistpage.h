#ifndef ESCROWLISTPAGE_H
#define ESCROWLISTPAGE_H

#include <QDialog>

namespace Ui {
    class EscrowListPage;
}
class EscrowTableModel;
class OptionsModel;
class WalletModel;
QT_BEGIN_NAMESPACE
class QTableView;
class QItemSelection;
class QSortFilterProxyModel;
class QMenu;
class QModelIndex;
class QKeyEvent;
QT_END_NAMESPACE

/** Widget that shows a list of owned escrowes.
  */
class EscrowListPage : public QDialog
{
    Q_OBJECT

public:
   

    explicit EscrowListPage(QWidget *parent = 0);
    ~EscrowListPage();


    void setModel(WalletModel*, EscrowTableModel *model);
    void setOptionsModel(OptionsModel *optionsModel);
    const QString &getReturnValue() const { return returnValue; }
    bool handleURI(const QString &uri);
	void keyPressEvent(QKeyEvent * event);
	void showEvent ( QShowEvent * event );
private:
    Ui::EscrowListPage *ui;
    EscrowTableModel *model;
    OptionsModel *optionsModel;
	WalletModel* walletModel;
    QString returnValue;
    QSortFilterProxyModel *proxyModel;
    QMenu *contextMenu;
    QAction *deleteAction; // to be able to explicitly disable it
    QString newEscrowToSelect;

private slots:
    void on_searchEscrow_clicked();
    /** Create a new escrow for receiving coins and / or add a new escrow book entry */
    /** Copy escrow of currently selected escrow entry to clipboard */
    void on_copyEscrow_clicked();
    /** Copy value of currently selected escrow entry to clipboard (no button) */
    void on_copyOffer_clicked();

    /** Export button clicked */
    void on_exportButton_clicked();

    /** Set button states based on selected tab and selection */
    void selectionChanged();
    /** Spawn contextual menu (right mouse menu) for escrow book entry */
    void contextualMenu(const QPoint &point);
    /** New entry/entries were added to escrow table */
    void selectNewEscrow(const QModelIndex &parent, int begin, int /*end*/);


};

#endif // ESCROWLISTPAGE_H
