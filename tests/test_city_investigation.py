"""Validate the authored street map without requiring the Windows game runtime.

This checks map structure and exercises its Record-target graph. It does not
replace dmap, collision/rendering checks, or an engine save/load playthrough.
Run: python -m unittest discover -s tests -v
"""
import copy
import itertools
import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAP = ROOT / 'stbrielle/maps/sb01_cityglimpse.map'


def parse_entities(text):
    tokens = re.findall(r'//[^\n]*|"(?:\\.|[^"\\])*"|[{}]|[^\s{}"]+', text)
    depth = 0
    entities = []
    current = None
    pending = None
    for token in tokens:
        if token.startswith('//'):
            continue
        if token == '{':
            if depth == 0:
                current = {}
            depth += 1
        elif token == '}':
            depth -= 1
            if depth < 0:
                raise ValueError('Extra closing brace')
            if depth == 0:
                if pending is not None:
                    raise ValueError('Unpaired entity key')
                entities.append(current)
        elif depth == 1 and token.startswith('"'):
            value = token[1:-1]
            if pending is None:
                pending = value
            else:
                if pending in current:
                    raise ValueError('Duplicate key: ' + pending)
                current[pending] = value
                pending = None
    if depth != 0:
        raise ValueError('Unclosed map entity/brush')
    return entities


def destinations(entity):
    # Game_local.GetTargets uses spawnArgs.MatchPrefix, in insertion order.
    return [v for k, v in entity.items() if k.startswith('target')]


class RecordGraph:
    """Small test driver for the map's set/check/notice/trigger target subset."""
    def __init__(self, state=None):
        self.entities = {e['name']: e for e in parse_entities(MAP.read_text()) if 'name' in e}
        self.state = dict(state or {})
        self.removed = set()
        self.notices = []

    def activate(self, name, depth=0):
        if depth > 50:
            raise AssertionError('Target cycle: ' + name)
        if name in self.removed:
            return
        e = self.entities[name]
        cls = e['classname']
        if cls == 'target_stbrielle_recordcheck':
            value = self.state.get(e['record_key'], '')
            compare = e['record_compare']
            truth = value not in ('', '0')
            passed = {'true': truth, 'false': not truth,
                      'equals': value.lower() == e.get('record_value', '1').lower()}[compare]
            if not passed:
                return
        elif cls == 'target_stbrielle_record':
            if e.get('record_mode', 'set') != 'set':
                raise AssertionError('Add interpreter support before testing another record mode')
            self.state[e['record_key']] = e.get('record_value', '1')
        elif cls == 'target_stbrielle_notice':
            self.notices.append(name)
        elif cls == 'light':
            return
        elif cls not in ('func_static', 'trigger_once'):
            raise AssertionError('Unsupported graph node: ' + cls)
        for target in destinations(e):
            self.activate(target, depth + 1)
        if e.get('record_once') == '1' or cls == 'trigger_once':
            self.removed.add(name)

    def read_evidence(self, order=('city_roster', 'city_testimony')):
        for name in order:
            self.activate(name)


class StreetInvestigationTests(unittest.TestCase):
    def test_all_targets_exist_and_names_are_unique(self):
        entities = parse_entities(MAP.read_text())
        names = [e['name'] for e in entities if 'name' in e]
        self.assertEqual(len(names), len(set(names)))
        for e in entities:
            for target in destinations(e):
                self.assertIn(target, names, e.get('name'))

    def test_all_axis_aligned_brushes_have_positive_volume(self):
        brushes = re.findall(r'brushDef3\s*\{([^{}]+)\}', MAP.read_text())
        self.assertGreater(len(brushes), 30)
        for i, brush in enumerate(brushes):
            planes = re.findall(r'\(\s*(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)\s*\)', brush)
            self.assertEqual(len(planes), 6, i)
            bounds = {tuple(map(int, p[:3])): int(p[3]) for p in planes}
            for axis in range(3):
                positive = tuple(1 if j == axis else 0 for j in range(3))
                negative = tuple(-1 if j == axis else 0 for j in range(3))
                self.assertLess(bounds[negative], -bounds[positive], (i, axis))

    def test_materials_and_entity_types_are_declared(self):
        materials = '\n'.join(p.read_text() for p in (ROOT / 'stbrielle/materials').glob('*.mtr'))
        definitions = '\n'.join(p.read_text() for p in (ROOT / 'stbrielle/def').glob('*.def'))
        for material in set(re.findall(r'"(textures/[^"\n]+)"', MAP.read_text())):
            self.assertIn(material, materials)
        for e in parse_entities(MAP.read_text()):
            self.assertRegex(definitions, r'entityDef\s+' + re.escape(e['classname']) + r'\s*\{')

    def test_dynamic_city_entities_exist(self):
        source = (ROOT / 'd3xp/sb_awareness.cpp').read_text()
        names = RecordGraph().entities
        for name in re.findall(r'FindEntity\( "(city[^\"]+)" \)', source):
            self.assertIn(name, names)

    def test_arrival_does_not_complete_investigation(self):
        game = RecordGraph({'sb_annex_filed_person_contact_count': '2'})
        game.activate('cityglimpse_arrival_trigger')
        self.assertEqual(game.state['sb_city_arrived'], '1')
        self.assertNotIn('sb_vertical_slice_complete', game.state)
        self.assertEqual(game.state['sb_annex_filed_person_contact_count'], '2')

    def test_dispatch_without_certification_is_denied(self):
        # Old exterior saves may already carry the legacy completion flag.
        # It must neither authorize a new signature nor grant dispatch.
        for seed in ({}, {'sb_vertical_slice_complete': '1'}):
            game = RecordGraph(seed)
            game.activate('city_dispatch')
            self.assertNotIn('sb_city_dispatch_received', game.state)
            self.assertEqual(game.notices, ['city_dispatch_denied_notice'])
            game.activate('city_certify_official')
            self.assertNotIn('sb_city_certification', game.state)

    def test_each_missing_evidence_combination_blocks_both_seals(self):
        for evidence in ((), ('city_roster',), ('city_testimony',)):
            for seal in ('official', 'witness'):
                with self.subTest(evidence=evidence, seal=seal):
                    game = RecordGraph()
                    game.read_evidence(evidence)
                    game.activate('city_certify_' + seal)
                    self.assertNotIn('sb_city_certification', game.state)
                    self.assertNotIn('sb_mercuria_intake_route', game.state)
                    self.assertTrue(game.notices[-1].startswith('city_missing_'))

    def test_both_evidence_orders_enable_either_complete_outcome(self):
        for order in itertools.permutations(('city_roster', 'city_testimony')):
            for seal, choice, route in (('official', '1', 'public'), ('witness', '2', 'service')):
                with self.subTest(order=order, seal=seal):
                    game = RecordGraph()
                    game.read_evidence(order)
                    game.activate('city_certify_' + seal)
                    self.assertEqual(game.state['sb_city_certification'], choice)
                    self.assertEqual(game.state['sb_mercuria_intake_route'], route)
                    self.assertNotIn('sb_vertical_slice_complete', game.state)
                    self.assertEqual(game.notices[-1], 'city_' + seal + '_notice')
                    game.activate('city_dispatch')
                    self.assertEqual(game.state['sb_city_investigation_complete'], '1')
                    self.assertEqual(game.state['sb_city_dispatch_received'], '1')
                    self.assertEqual(game.state['sb_vertical_slice_complete'], '1')
                    self.assertEqual(game.notices[-1], 'city_dispatch_' + seal + '_notice')

    def test_first_certification_wins_even_before_director_tick(self):
        for first, second in (('official', 'witness'), ('witness', 'official')):
            game = RecordGraph()
            game.read_evidence()
            game.activate('city_certify_' + first)
            signed = dict(game.state)
            game.activate('city_certify_' + second)
            self.assertEqual(game.state, signed)
            self.assertEqual(game.notices[-1], 'city_' + first + '_notice')

    def test_evidence_and_receipts_are_repeatable_without_changing_record(self):
        game = RecordGraph({'sb_awareness': '11', 'sb_record_stability': '70'})
        game.read_evidence()
        game.activate('city_certify_witness')
        game.activate('city_dispatch')
        before = dict(game.state)
        for _ in range(10):
            game.read_evidence()
            game.activate('city_certify_official')
            game.activate('city_dispatch')
        self.assertEqual(game.state, before)
        self.assertEqual(game.notices[-1], 'city_dispatch_witness_notice')

    def test_progress_survives_record_roundtrip_and_map_revisit(self):
        for seal in ('official', 'witness'):
            game = RecordGraph()
            game.activate('city_roster')
            game = RecordGraph(json.loads(json.dumps(game.state)))
            game.activate('city_testimony')
            game.activate('city_certify_' + seal)
            signed = dict(game.state)
            game = RecordGraph(json.loads(json.dumps(game.state)))
            game.activate('city_certify_' + ('witness' if seal == 'official' else 'official'))
            self.assertEqual(game.state, signed)
            game.activate('city_dispatch')
            game = RecordGraph(json.loads(json.dumps(game.state)))
            before = dict(game.state)
            game.activate('city_dispatch')
            self.assertEqual(game.state, before)

    def test_all_reachable_interaction_states_obey_progression_contract(self):
        actions = ('city_roster', 'city_testimony', 'city_certify_official', 'city_certify_witness', 'city_dispatch')
        queue = [RecordGraph()]
        visited = set()
        while queue:
            game = queue.pop()
            key = tuple(sorted(game.state.items()))
            if key in visited:
                continue
            visited.add(key)
            for action in actions:
                next_game = copy.deepcopy(game)
                next_game.activate(action)
                state = next_game.state
                if 'sb_city_certification' in state:
                    self.assertEqual(state['sb_city_roster_read'], '1')
                    self.assertEqual(state['sb_city_testimony_read'], '1')
                    self.assertEqual(state['sb_mercuria_intake_route'], {'1': 'public', '2': 'service'}[state['sb_city_certification']])
                if 'sb_city_certification' in game.state:
                    self.assertEqual(state['sb_city_certification'], game.state['sb_city_certification'])
                if 'sb_vertical_slice_complete' in state:
                    self.assertEqual(state['sb_city_dispatch_received'], '1')
                    self.assertIn('sb_city_certification', state)
                queue.append(next_game)
        self.assertGreaterEqual(len(visited), 8)


if __name__ == '__main__':
    unittest.main()
