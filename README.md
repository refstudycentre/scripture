# scripture
A Drupal module which provides field types, formatters, widgets and a DB-backend for referencing verses of the Bible in different translations. Living Word heavily depends on this module.

## What it does:

### scripture.install

- Define database schema to store bible translations, books and verses
- Define database schema for a field type containing a range of verses

### scripture.module

- Define permissions
- Define menu callbacks
- Define a "verse picker" form element
- Functions to retrieve and display verses
- Function to manipulate and display verse ranges

### scripture.pages.inc

Shows all nodes associated with a given verse range

### scripture.admin.inc

Defines back end pages

### scripture.db.inc

A bunch of database getter functions

### scripture.element.inc

Contains all of the functionality of the "verse picker" form element

### scripture.field.inc

Define field types and instances for this module.
