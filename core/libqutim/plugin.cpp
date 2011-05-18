/****************************************************************************
 *  plugin.cpp
 *
 *  Copyright (c) 2010 by Nigmatullin Ruslan <euroelessar@gmail.com>
 *
 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*****************************************************************************/

#include "plugin_p.h"
#include "objectgenerator_p.h"
#include <QtCore/QCoreApplication>

namespace qutim_sdk_0_3
{
	PluginInfoData::PluginInfoData()
	    : version(PLUGIN_VERSION(0, 0, 0, 1)), inited(0), loaded(0)
	{
	}

	PluginInfoData::PluginInfoData(const PluginInfoData &o)
	    : QSharedData(o), authors(o.authors), name(o.name), description(o.description),
	      version(o.version), inited(o.inited), loaded(o.loaded),
	      capabilities(o.capabilities), icon(o.icon)
	{
	}

	PluginInfo::PluginInfo(const LocalizedString &name, const LocalizedString &description,
						   quint32 version, ExtensionIcon icon)
	{
		d = new PluginInfoData;
		setName(name);
		setDescription(description);
		setVersion(version);
		setIcon(icon);
	}

	PluginInfo::PluginInfo(const PluginInfo &other) : d(other.d)
	{
	}

	PluginInfo::~PluginInfo()
	{
	}

	PluginInfo &PluginInfo::operator =(const PluginInfo &other)
	{
		d = other.d;
		return *this;
	}

	PluginInfo &PluginInfo::addAuthor(const PersonInfo &author)
	{
		d->authors.append(author);
		return *this;
	}

	PluginInfo &PluginInfo::addAuthor(const QString &ocsUsername)
	{
		return addAuthor(PersonInfo(ocsUsername));
	}

	PluginInfo &PluginInfo::addAuthor(const LocalizedString &name, const LocalizedString &task,
									  const QString &email, const QString &web)
	{
		return addAuthor(PersonInfo(name, task, email, web));
	}

	PluginInfo &PluginInfo::setName(const LocalizedString &name)
	{
		d->name = name;
		return *this;
	}

	PluginInfo &PluginInfo::setDescription(const LocalizedString &description)
	{
		d->description = description;
		return *this;
	}

	PluginInfo &PluginInfo::setIcon(const ExtensionIcon &icon)
	{
		d->icon = icon;
		return *this;
	}

	PluginInfo &PluginInfo::setVersion(quint32 version)
	{
		d->version = version;
		return *this;
	}
	
	PluginInfo::Capabilities PluginInfo::capabilities() const
	{
		return d->capabilities;
	}
	
	PluginInfo &PluginInfo::setCapabilities(PluginInfo::Capabilities capabilities)
	{
		d->capabilities = capabilities;
		return *this;
	}

	QList<PersonInfo> PluginInfo::authors() const
	{
		return d->authors;
	}

	LocalizedString PluginInfo::name() const
	{
		return d->name;
	}

	LocalizedString PluginInfo::description() const
	{
		return d->description;
	}


	quint32 PluginInfo::version() const
	{
		return d->version;
	}

	ExtensionIcon PluginInfo::icon() const
	{
		return d->icon;
	}

	PluginInfo::Data *PluginInfo::data()
	{
		return const_cast<Data *>(d.constData());
	}

	QString PluginInfo::fileName() const
	{
		return d->fileName;
	}

	Plugin::Plugin() : p(new PluginPrivate)
	{
	}

	Plugin::~Plugin()
	{
	}

	PluginInfo Plugin::info() const
	{
		return p->info;
	}

	ExtensionInfoList Plugin::avaiableExtensions() const
	{
		return p->extensions;
	}

	void Plugin::addAuthor(const LocalizedString &name, const LocalizedString &task,
						   const QString &email, const QString &web)
	{
		if (p->info.data()->inited)
			return;
		Q_UNUSED(p->info.addAuthor(name, task, email, web));
	}
	
	void Plugin::addAuthor(const QString &ocsUsername)
	{
		if (p->info.data()->inited)
			return;
		p->info.addAuthor(ocsUsername);
	}

	void Plugin::setInfo(const LocalizedString &name, const LocalizedString &description,
						 quint32 version, ExtensionIcon icon)
	{
		if (p->info.data()->inited)
			return;
		p->info.setName(name);
		p->info.setDescription(description);
		p->info.setVersion(version);
		p->info.setIcon(icon);
	}
	
	void Plugin::setCapabilities(Capabilities capabilities)
	{
		if (p->info.data()->inited)
			return;
		p->info.setCapabilities(static_cast<PluginInfo::Capabilities>(int(capabilities)));
	}

	void Plugin::addExtension(const LocalizedString &name, const LocalizedString &description,
							  const ObjectGenerator *generator, ExtensionIcon icon)
	{
		Q_ASSERT_X(!p->info.data()->inited, "Plugin::addExtension", "This method must be called only from Plugin::init()");
		if (p->info.data()->inited)
			return;
		Q_ASSERT_X(generator->metaObject(), "Plugin::addExtension", "ObjectGenerator must contain QMetaObject");
		p->extensions << ExtensionInfo(name, description, generator, icon);
	}

	// TODO: Add validation :)
	bool PluginPrivate::validate()
	{
//		for (int i = 0; i < extensions.size(); i++) {
//			ExtensionInfo &extensionInfo = extensions[i];
//			extensionInfo.data()->plugin = info;
//			const_cast<ObjectGenerator *>(extensionInfo.data()->gen)->data()->info = extensionInfo;
//		}
		return true;
	}
}