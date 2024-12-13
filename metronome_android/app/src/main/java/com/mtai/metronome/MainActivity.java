package com.mtai.metronome;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;

import android.os.Bundle;

import com.google.android.material.tabs.TabLayout;
import com.mtai.metronome.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {
    MetronomeFragment metronomeFragment = new MetronomeFragment();
    TunerFragment tunerFragment = new TunerFragment();
    private ActivityMainBinding binding;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        initTabLayout();
    }

    private void initTabLayout(){
        TabLayout tabLayout = binding.tabLayout;
        tabLayout.addOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                Fragment fragment;
                if (tab.getPosition() == 1) {
                    fragment = tunerFragment;
                } else {
                    fragment = metronomeFragment;
                }
                getSupportFragmentManager().beginTransaction().replace(R.id.fragmentContainerView,fragment).commit();
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {
            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {
            }
        });
    }


}