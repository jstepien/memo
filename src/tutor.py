from activerecord import ActiveRecord
from entities import Test, Question

# Optimising the following monstrosity somehow seems like a good idea.
TestPreparationQuery = '''select ok.id as id, 0 as inverted,
		(max(wrong.count) + 1.) / (max(ok.count) + 1.) as ratio from (
		select p.id as id, count(result) as count from pairs p
		join questions q on q.pair_id = p.id
		where inverted = 0 and result = 0
		group by pair_id
		union
		select id, 0 as count from pairs
	) wrong join (
		select p.id as id, count(result) as count from pairs p
		join questions q on q.pair_id = p.id
		where inverted = 0 and result = 1
		group by pair_id
		union
		select id, 0 as count from pairs
	) ok on ok.id = wrong.id group by ok.id
	union select ok.id as id, 1 as inverted,
	(max(wrong.count) + 1.) / (max(ok.count) + 1.) as ratio from (
		select p.id as id, count(result) as count from pairs p
		join questions q on q.pair_id = p.id
		where inverted = 1 and result = 0
		group by pair_id
		union
		select id, 0 as count from pairs
	) wrong join (
		select p.id as id, count(result) as count from pairs p
		join questions q on q.pair_id = p.id
		where inverted = 1 and result = 1
		group by pair_id
		union
		select id, 0 as count from pairs
	) ok on ok.id = wrong.id group by ok.id
	order by ratio desc;
	'''

def prepare_test():
	t = Test()
	questions = [Question(t, row[0], inverted=(row[1] == 1)) for row in
			ActiveRecord.execute(TestPreparationQuery)]
	t.save()
	return t
