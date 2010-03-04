import re
from storm.locals import Store, create_database
import storm.exceptions

class ActiveRecord(object):
	'''Provides some methods which implement the Active Record pattern.
	Additionally stores a reference to a Storm store.'''
	store = None

	subclasses = []

	def save(self):
		ActiveRecord.store.add(self)

	@classmethod
	def find(cls, *args, **kwargs):
		return ActiveRecord.store.find(cls, *args, **kwargs)

	@staticmethod
	def connect(connection_string):
		'''Connects to a given database and prepares it's structure.'''
		database = create_database(connection_string)
		ActiveRecord.store = Store(database)
		for type in ActiveRecord.subclasses:
			schema = type.table_schema
			if isinstance(schema, str) or isinstance(schema, unicode):
				ActiveRecord.store.execute(type.table_schema)
			else:
				for statement in schema:
					ActiveRecord.store.execute(statement)

	@staticmethod
	def register_subclass(subclass):
		ActiveRecord.subclasses.append(subclass)

	@staticmethod
	def commit():
		try:
			ActiveRecord.store.commit()
		except storm.exceptions.IntegrityError as ex:
			if re.match('columns? [a-zA-Z_, ]+ (is|are) not unique', str(ex)):
				raise NonUniqueColumnError()
			elif re.match('^[\.a-zA-Z_, ]+ may not be NULL', str(ex)):
				raise NullException()
			else:
				raise ex

	@staticmethod
	def execute(statement):
		return ActiveRecord.store.execute(statement)

class NonUniqueColumnError(Exception):
	pass

class NullException(Exception):
	pass
