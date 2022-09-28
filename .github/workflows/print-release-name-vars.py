import argparse
import subprocess

def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0', ''):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

parser = argparse.ArgumentParser()
parser.add_argument('--github_org', required=True)
parser.add_argument('--full_release', required=True, type=str2bool)
parser.add_argument('--version_type', required=True)
parser.add_argument('--formatted_time', required=True)
parser.add_argument('--number')
args = parser.parse_args()

if args.number:
    args.number = int(args.number)

release_tag_name = ''
release_name = ''

if args.full_release:
    release_tag_name = f'2.55-{args.version_type}-{args.number}'
    release_name = f'2.55 {args.version_type.capitalize()} {args.number}'
else:
    i = 1
    while True:
        release_tag_name = f'nightly-{args.formatted_time}'
        release_name = f'Nightly {args.formatted_time}'
        if i != 1:
            release_tag_name += f'-{i}'
            release_name += f' ({i})'
        proc = subprocess.run(['git', 'rev-parse', release_tag_name])
        if proc.returncode == 0:
            i += 1
        else:
            break

if args.github_org != 'ArmageddonGames':
    release_tag_name = f'{args.github_org}-{release_tag_name}'

print(f'::set-output name=release_tag_name::{release_tag_name}')
print(f'::set-output name=release_name::{release_name}')