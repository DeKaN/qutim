/****************************************************************************
 *  message.cpp
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

#include "message.h"
#include <QDateTime>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include "chatunit.h"
#include "account.h"
#include "protocol.h"

namespace qutim_sdk_0_3
{
	class QmlMessage : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(QString text READ text WRITE setText NOTIFY onTextChanged)
		Q_PROPERTY(QDateTime dateTime READ dateTime WRITE setDateTime NOTIFY onDateTimeChanged)
		Q_PROPERTY(bool incoming READ isIncoming WRITE setIncoming NOTIFY onIncomingChanged)
		Q_PROPERTY(QObject* chatUnit READ chatUnit WRITE setChatUnit NOTIFY onChatUnitChanged)
		Q_PROPERTY(quint64 id READ id CONSTANT)
	public:
		QmlMessage(Message &message, QObject *parent = 0) : QObject(parent), m_message(&message), m_delete(false)
		{
			foreach(const QByteArray &name, message.dynamicPropertyNames())
				setProperty(name, message.property(name));
		}
		QmlMessage(QObject *parent = 0) : QObject(parent), m_message(new Message), m_delete(true) {}
		~QmlMessage() { if(m_delete) delete m_message; }
		Message message() const { return *m_message; }

		QString text() { return m_message->text(); }
		QDateTime dateTime() { return m_message->time(); }
		bool isIncoming() { return m_message->isIncoming(); }
		QObject *chatUnit() { return m_message->chatUnit(); }
		quint64 id() { return m_message->id(); }

		void setText(const QString &t)
		{
			m_message->setText(t);
			emit onTextChanged(t);
		}
		void setDateTime(const QDateTime &dt)
		{
			m_message->setTime(dt);
			emit onDateTimeChanged(dt);
		}
		void setIncoming(bool i)
		{
			m_message->setIncoming(i);
			emit onIncomingChanged(i);
		}
		void setChatUnit(QObject *obj)
		{
			m_message->setChatUnit(qobject_cast<ChatUnit *>(obj));
			emit onChatUnitChanged(obj);
		}

	protected:
		bool event(QEvent *ev)
		{
			if (ev->type() == QEvent::DynamicPropertyChange) {
				QDynamicPropertyChangeEvent *event = static_cast<QDynamicPropertyChangeEvent *>(ev);
				m_message->setProperty(event->propertyName(), property(event->propertyName()));
			}
		}

	signals:
		void onTextChanged(const QString &);
		void onDateTimeChanged(const QDateTime &);
		void onIncomingChanged(bool);
		void onChatUnitChanged(QObject *);

	private:
		bool m_delete;
		Message *m_message;
	};

	QScriptValue messageToScriptValue(QScriptEngine *engine, const Message &mes)
	{
		QScriptValue obj = engine->newObject();
		obj.setProperty("time", engine->newDate(mes.time()));
		obj.setProperty("chatUnit", engine->newQObject(const_cast<ChatUnit *>(mes.chatUnit())));
		obj.setProperty("text", mes.text());
		obj.setProperty("in", mes.isIncoming());
		foreach(const QByteArray &name, mes.dynamicPropertyNames())
			obj.setProperty(QString::fromUtf8(name), engine->newVariant(mes.property(name)));
		return obj;
	}

	void messageFromScriptValue(const QScriptValue &obj, Message &mes)
	{
		QScriptValueIterator it(obj);
		while (it.hasNext()) {
			it.next();
			mes.setProperty(it.name().toUtf8(), it.value().toVariant());
		}
	}

	void Message::scriptRegister(QScriptEngine *engine)
	{
		qScriptRegisterMetaType(engine, &messageToScriptValue, &messageFromScriptValue);
	}

	static quint64 message_id = 0;

	class MessagePrivate : public QSharedData
	{
	public:
		MessagePrivate() : in(false), chatUnit(0), id(++message_id) { text.clear(); }
		MessagePrivate(const MessagePrivate &o)
				: QSharedData(o), text(o.text), time(o.time),
				in(o.in), chatUnit(o.chatUnit), id(++message_id),
				names(o.names), values(o.values) {}
		~MessagePrivate() {}
		QString text;
		QDateTime time;
		bool in;
		ChatUnit *chatUnit;
		quint64 id;
		QVariant getText() const { return text; }
		void setText(const QVariant &val) { text = val.toString(); }
		QVariant getTime() const { return time; }
		void setTime(const QVariant &val) { time = val.toDateTime(); }
		QVariant getIn() const { return in;}
		void setIn(const QVariant &input) { in = input.toBool(); }
		void setChatUnit (const QVariant &val) { chatUnit = val.value<ChatUnit *>(); }
		QVariant getChatUnit() const { return QVariant::fromValue(chatUnit); }
		QList<QByteArray> names;
		QList<QVariant> values;
	};

	namespace CompiledProperty
	{
		typedef QVariant (MessagePrivate::*Getter)() const;
		typedef void (MessagePrivate::*Setter)(const QVariant &variant);
		static QList<QByteArray> names = QList<QByteArray>()
										 << "text"
										 << "time"
										 << "in"
										 << "chatUnit";
		static QList<Getter> getters   = QList<Getter>()
										 << &MessagePrivate::getText
										 << &MessagePrivate::getTime
										 << &MessagePrivate::getIn
										 << &MessagePrivate::getChatUnit;
		static QList<Setter> setters   = QList<Setter>()
										 << &MessagePrivate::setText
										 << &MessagePrivate::setTime
										 << &MessagePrivate::setIn
										 << &MessagePrivate::setChatUnit;
	}

	Message::Message() : p(new MessagePrivate)
	{
	}

	Message::Message(const QString &text) : p(new MessagePrivate)
	{
		p->text = text;
	}

	Message::Message(const Message &other) : p(other.p)
	{
	}

	Message::~Message()
	{
	}

	Message &Message::operator =(const Message &other)
	{
		p = other.p;
		return *this;
	}

	QVariant Message::property(const char *name) const
	{
		return property(name, QVariant());
	}

	QVariant Message::property(const char *name, const QVariant &def) const
	{
		QByteArray prop = QByteArray::fromRawData(name, strlen(name));
		int id = CompiledProperty::names.indexOf(prop);
		if(id < 0)
		{
			id = p->names.indexOf(prop);
			if(id < 0)
				return def;
			return p->values.at(id);
		}
		return (p->*CompiledProperty::getters.at(id))();
	}

	void Message::setProperty(const char *name, const QVariant &value)
	{
		QByteArray prop = QByteArray::fromRawData(name, strlen(name));
		int id = CompiledProperty::names.indexOf(prop);
		if(id < 0)
		{
			id = p->names.indexOf(prop);
			if(!value.isValid())
			{
				if(id < 0)
					return;
				p->names.removeAt(id);
				p->values.removeAt(id);
			}
			else
			{
				if(id < 0)
				{
					prop.detach();
					p->names.append(prop);
					p->values.append(value);
				}
				else
					p->values[id] = value;
			}
		}
		else
			(p->*CompiledProperty::setters.at(id))(value);
	}

	QList<QByteArray> Message::dynamicPropertyNames() const
	{
		return p->names;
	}

	const QString &Message::text() const
	{
		return p->text;
	}

	void Message::setText(const QString &text)
	{
		p->text = text;
	}

	const QDateTime &Message::time() const
	{
		return p->time;
	}

	void Message::setTime(const QDateTime &time)
	{
		p->time = time;
	}

	bool Message::isIncoming() const
	{
		return p->in;
	}

	void Message::setIncoming(bool input)
	{
		p->in = input;
	}

	
	const ChatUnit* Message::chatUnit() const
	{
		return p->chatUnit;
	}

	quint64 Message::id() const
	{
		return p->id;
	}
	
	void Message::setChatUnit ( ChatUnit* chatUnit )
	{
		p->chatUnit = chatUnit;
	}

	MessageReceiptEvent::MessageReceiptEvent(quint64 id, bool success) :
			QEvent(eventType()), i(id), s(success)
	{
	}

	QEvent::Type MessageReceiptEvent::eventType()
	{
		static QEvent::Type type = QEvent::Type(QEvent::registerEventType(QEvent::User + 101));
		return type;
	}
}
