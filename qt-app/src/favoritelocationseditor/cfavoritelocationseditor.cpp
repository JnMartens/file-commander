#include "cfavoritelocationseditor.h"
#include "ui_cfavoritelocationseditor.h"

#include "cnewfavoritelocationdialog.h"
#include "ccontroller.h"

// If an item represents a subcategory, it cannot link to a location and can only be used as a container
class CFavoriteLocationsListItem : public QTreeWidgetItem
{
public:
	CFavoriteLocationsListItem(QTreeWidget * parent, std::list<CLocationsCollection>& parentList, std::list<CLocationsCollection>::iterator& dataItemIterator, bool isCategory) :
		QTreeWidgetItem(parent, QStringList() << dataItemIterator->displayName),
		_itemIterator(dataItemIterator),
		_parentList(parentList),
		_bIsCategory(isCategory)
	{
		if (isCategory)
			setBold();
	}

	CFavoriteLocationsListItem(QTreeWidgetItem * parentItem, std::list<CLocationsCollection>& parentList, std::list<CLocationsCollection>::iterator& dataItemIterator, bool isCategory) :
		QTreeWidgetItem(parentItem, QStringList() << dataItemIterator->displayName),
		_itemIterator(dataItemIterator),
		_parentList(parentList),
		_bIsCategory(isCategory)

	{
		if (isCategory)
			setBold();
	}

	std::list<CLocationsCollection>::iterator itemIterator() const {return _itemIterator;}
	std::list<CLocationsCollection>& list() {return _parentList;}
	bool isCategory() const {return _bIsCategory;}

private:
	void setBold()
	{
		QFont newFont(font(0));
		newFont.setBold(true);
		setFont(0, newFont);
	}

private:
	std::list<CLocationsCollection>::iterator _itemIterator;
	std::list<CLocationsCollection>& _parentList;
	bool _bIsCategory;
};

CFavoriteLocationsEditor::CFavoriteLocationsEditor(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CFavoriteLocationsEditor),
	_locations(CController::get().favoriteLocations())
{
	ui->setupUi(this);

	connect(ui->_list, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(ui->_list, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(contextMenu(const QPoint &)));

	fillUI();
}

CFavoriteLocationsEditor::~CFavoriteLocationsEditor()
{
	delete ui;
}

void CFavoriteLocationsEditor::currentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{

}

void CFavoriteLocationsEditor::contextMenu(const QPoint & pos)
{
	CFavoriteLocationsListItem * item = dynamic_cast<CFavoriteLocationsListItem*>(ui->_list->itemAt(pos));
	QMenu menu;
	QAction * addItemAction = 0;
	if (!item || item->isCategory())
		addItemAction = menu.addAction("Add item...");
	else
	{
		addItemAction = menu.addAction("You can only add nested items to categories, not to end items");
		addItemAction->setEnabled(false);
	}

	connect(addItemAction, &QAction::triggered, [this, item](){
		CNewFavoriteLocationDialog dialog(this, false);
		if (dialog.exec() == QDialog::Accepted)
		{
			std::list<CLocationsCollection>& list = item ? item->itemIterator()->subLocations : _locations.locations();
			list.emplace_back(CLocationsCollection(dialog.name(), dialog.location()));
			if (item)
			{
				new CFavoriteLocationsListItem(item, list, --list.end(), false);
				item->setExpanded(true);
			}
			else
				new CFavoriteLocationsListItem(ui->_list, list, --list.end(), false);
		}
	});

	connect(menu.addAction("Add category..."), &QAction::triggered, [this, item](){
		CNewFavoriteLocationDialog dialog(this, true);
		if (dialog.exec() == QDialog::Accepted)
		{
			std::list<CLocationsCollection>& list = item ? item->itemIterator()->subLocations : _locations.locations();
			list.emplace_back(CLocationsCollection(dialog.name()));
			if (item)
			{
				new CFavoriteLocationsListItem(item, list, --list.end(), true);
				item->setExpanded(true);
			}
			else
				new CFavoriteLocationsListItem(ui->_list, list, --list.end(), true);
		}
	});

	if (item)
	{
		connect(menu.addAction("Remove item"), &QAction::triggered, [this, item]() {
			if (item->isCategory())
			{
				if (QMessageBox::question(this, "Delete the item?", "Are you sure you want to delete this category and all its sub-items?") == QMessageBox::Yes)
				{
					item->list().erase(item->itemIterator());
					delete item;
				}
			}
			else
			{
				item->list().erase(item->itemIterator());
				delete item;
			}
		});
	}

	menu.exec(ui->_list->mapToGlobal(pos));
}

void CFavoriteLocationsEditor::fillUI()
{
	ui->_list->clear();
	for (auto it = _locations.locations().begin(); it != _locations.locations().end(); ++it)
		addLocationsToTreeWidget(_locations.locations(), it, 0);
}

void CFavoriteLocationsEditor::addLocationsToTreeWidget(std::list<CLocationsCollection>& parentList, std::list<CLocationsCollection>::iterator& locationCollectionListIterator, QTreeWidgetItem* parent)
{
	assert(locationCollectionListIterator->subLocations.empty() || locationCollectionListIterator->absolutePath.isEmpty());
	const bool isCategory = locationCollectionListIterator->absolutePath.isEmpty();
	CFavoriteLocationsListItem * treeWidgetItem = parent ? new CFavoriteLocationsListItem(parent, parentList, locationCollectionListIterator, isCategory) : new CFavoriteLocationsListItem(ui->_list, parentList, locationCollectionListIterator, isCategory);
	if (!locationCollectionListIterator->subLocations.empty())
		for (auto it = locationCollectionListIterator->subLocations.begin(); it != locationCollectionListIterator->subLocations.end(); ++it)
			addLocationsToTreeWidget(locationCollectionListIterator->subLocations, it, treeWidgetItem);
}
