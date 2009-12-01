/*********************************************************************
* Copyright 2009 Alan Buckley
*
* This file is part of PackIt.
*
* PackIt is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* PackIt is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with PackIt. If not, see <http://www.gnu.org/licenses/>.
*
*****************************************************************************/


#ifndef binding_h
#define binding_h

#include "Packager.h"
#include "tbx/writeablefield.h"
#include "tbx/textchangedlistener.h"
#include "tbx/numberrange.h"
#include "tbx/valuechangedlistener.h"
#include "tbx/stringset.h"
#include "tbx/stringutils.h"

class TextEndPoint;

class BindingUpdater
{
protected:
    TextEndPoint *_target;

public:
    BindingUpdater(TextEndPoint *target) : _target(target)
    {
    }
};

class TextEndPoint
{
   public:
      virtual ~TextEndPoint() {}
      virtual std::string get() const = 0;
      virtual void set(std::string value) = 0;
      virtual BindingUpdater *get_updater(TextEndPoint *target) {return 0;}
};

/**
 * Endpoint to get or set a packager field
 */
class PackagerTextEndPoint : public TextEndPoint
{
   Packager *_packager;

   std::string (Packager::*_get)() const;
   void (Packager::*_set)(std::string);

   public:
      PackagerTextEndPoint(Packager *packager,
            std::string (Packager::*get)() const,
            void (Packager::*set)(std::string)
                ) : _packager(packager), _get(get), _set(set)
      {
      }

     std::string get() const {return (_packager->*_get)();}
     void set(std::string value) {(_packager->*_set)(value);}
};

/**
 * Updater to update a target from a writeable field
 */
class WriteableBindingUpdater : public BindingUpdater, public tbx::TextChangedListener
{
    public:
       WriteableBindingUpdater(TextEndPoint *target) : BindingUpdater(target)
       {
       };

       virtual void text_changed(tbx::TextChangedEvent &event)
       {
    	   printf("text changed to %s\n", event.text().c_str());
          _target->set(event.text());
       }
};

/**
 * WriteableField end point
 */
class WriteableTextEndPoint : public TextEndPoint
{
    tbx::WriteableField _writeable;

public:
     WriteableTextEndPoint(tbx::WriteableField w) : _writeable(w) {}

     std::string get() const {return _writeable.text();}
     void set(std::string value) {_writeable.text(value);}

     virtual BindingUpdater *get_updater(TextEndPoint *target)
     {
    	 WriteableBindingUpdater *updater = new WriteableBindingUpdater(target);
    	 _writeable.add_text_changed_listener(updater);
    	 return updater;
     }

};

/**
 * Updater to update a target from a number range
 */
class NumberRangeBindingUpdater : public BindingUpdater, public tbx::ValueChangedListener
{
    public:
       NumberRangeBindingUpdater(TextEndPoint *target) : BindingUpdater(target)
       {
       };

       virtual void value_changed(tbx::ValueChangedEvent &event)
       {
         _target->set(tbx::to_string(event.value()));
       }
};

/**
 * NumberRange End point
 */
class NumberRangeEndPoint : public TextEndPoint
{
    tbx::NumberRange _numberrange;

public:
     NumberRangeEndPoint(tbx::NumberRange n) : _numberrange(n) {}

     std::string get() const {return tbx::to_string(_numberrange.value());}
     void set(std::string value) {_numberrange.value(tbx::from_string<int>(value));}

     virtual BindingUpdater *get_updater(TextEndPoint *target)
     {
    	 NumberRangeBindingUpdater *updater = new NumberRangeBindingUpdater(target);
    	 _numberrange.add_value_changed_listener(updater);
    	 return updater;
     }

};

/**
 * StringSet End point
 */
class StringSetEndPoint : public TextEndPoint
{
    tbx::StringSet _stringset;

public:
     StringSetEndPoint(tbx::StringSet s) : _stringset(s) {}

     std::string get() const {return _stringset.selected();}
     void set(std::string value) {_stringset.selected(value);}

     virtual BindingUpdater *get_updater(TextEndPoint *target)
     {
    	 // Can use Writeable binding as it uses the same text changed event
    	 WriteableBindingUpdater *updater = new WriteableBindingUpdater(target);
    	 _stringset.add_text_changed_listener(updater);
    	 return updater;
     }

};

/**
 * Binding between to text end points, from source to target
 */
class TextBinding
{
     TextEndPoint *_source;
     TextEndPoint *_target;
     BindingUpdater *_updater;
public:
     TextBinding(TextEndPoint *source, TextEndPoint *target) :
        _source(source), _target(target)
     {
        _updater = _source->get_updater(_target);
     }

     void update_target()
     {
        _target->set(_source->get());
     }

     void update_source()
     {
        _source->set(_target->get());
     }
};


#endif
