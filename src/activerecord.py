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
	def find(cls):
		return ActiveRecord.store.find(cls)

	@staticmethod
	def connect(connection_string):
		'''Connects to a given database and prepares it's structure.'''
		database = create_database(connection_string)
		ActiveRecord.store = Store(database)
		for type in ActiveRecord.subclasses:
			ActiveRecord.store.execute(type.table_schema)

	@staticmethod
	def register_subclass(subclass):
		ActiveRecord.subclasses.append(subclass)

	@staticmethod
	def commit():
		try:
			ActiveRecord.store.commit()
		except storm.exceptions.IntegrityError as ex:
			if str(ex).find('column value is not unique') != -1:
				raise NonUniqueColumnValueError()
			else:
				raise ex

class NonUniqueColumnValueError(Exception):
	pass
